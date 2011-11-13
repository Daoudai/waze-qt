/* roadmap_tile_storage_qtsql.cc - Tiles storage management using qtsqlite engine
 *
 * LICENSE:
 *
 *   Copyright 2011 Assaf Paz
 *
 *   This file is part of Waze.
 *
 *   Waze is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Waze is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Waze; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <QtSql>

extern "C" {
    #include "roadmap.h"
    #include "roadmap_tile_storage.h"
    #include "roadmap_locator.h"
    #include "roadmap_performance.h"
    #include "roadmap_file.h"
    #include "roadmap_path.h"
    #include "roadmap_main.h"
}

typedef enum
{
	_con_lifetime_session = 0x0,		// Connection remains active only within one statement execution
        _con_lifetime_application			// Connection remains active within application
} RMTileStorageConLifetime;


#define	  RM_TILE_STORAGE_QUERY_MAXSIZE 		1024
#define	  RM_TILE_STORAGE_DB_NAME_SIZE 			32
#define	  RM_TILE_STORAGE_DB_PATH_MAXSIZE 		512
#define	  RM_TILE_STORAGE_TRANS_TIMEOUT			2000L			// Transaction timeout in msec
#define	  RM_TILE_STORAGE_TRANS_STMTS_CNT		200			// Transaction timeout in msec
#define   RM_TILE_STORAGE_DB_PREFIX 			"tiles_"
#define   RM_TILE_STORAGE_DB_SUFFIX 			".db"
#define   RM_TILE_STORAGE_TILES_TABLE 			"tiles_table"
#define   RM_TILE_STORAGE_TILES_TABLE_ID		"id"
#define   RM_TILE_STORAGE_TILES_TABLE_DATA		"data"

#define   RM_TILE_STORAGE_STMT_CREATE_TABLE		"CREATE TABLE IF NOT EXISTS tiles_table(id INTEGER PRIMARY KEY, data BLOB)"
#define   RM_TILE_STORAGE_STMT_STORE		    "INSERT OR REPLACE INTO tiles_table values (%d,?);"
#define   RM_TILE_STORAGE_STMT_LOAD		        "SELECT data FROM tiles_table WHERE id=?;"
#define   RM_TILE_STORAGE_STMT_REMOVE	        "DELETE FROM tiles_table WHERE id=?;"
#define   RM_TILE_STORAGE_STMT_SYNC_OFF 		"PRAGMA synchronous = OFF"
#define   RM_TILE_STORAGE_STMT_CNT_OFF			"PRAGMA count_changes = OFF"
#define   RM_TILE_STORAGE_STMT_TMP_STORE_MEM	"PRAGMA temp_store = MEMORY"
#define   RM_TILE_STORAGE_STMT_CACHE_SIZE		"PRAGMA cache_size = 2000"
#define   RM_TILE_STORAGE_STMT_PAGE_SIZE		"PRAGMA page_size = 8192"

static int sgCurrentFips	= -1;			// Current fips - used to avoid database
static BOOL sgTableExists   = FALSE;		// Indicates if the table exits ( in order to avoid unnecessary queries)
static QSqlDatabase* sgSQLiteDb  = NULL;			// The current db handle
static RMTileStorageConLifetime sgConLifetime = _con_lifetime_session;		// The type of connection lifetime

static BOOL sgIsInTransaction = FALSE;
static int  sgTransStmtsCount = 0;

#define check_sqlite_error( errstr, code ) \
	check_sqlite_error_line( errstr, code, __LINE__ )

static void trans_timeout( void );
static void trans_commit( void );

/***********************************************************/
/*  Name        : roadmap_camera_image_capture()
 *  Purpose     : Auxiliary function. Returns the full path of the global tile file.
 *                  Pointer to the statically allocated memory is returned
 *  Params		: [in] fips
 *				:
 */
static const char * get_global_filename( int fips ) {

   const char *map_path = roadmap_db_map_path ();
   char name[30];
   static char filename[RM_TILE_STORAGE_DB_PATH_MAXSIZE] = {0};

     /* Global square id */
   if ( !filename[0])
   {
	   const char *suffix = "index";
	   snprintf (name, sizeof (name), "%05d_%s%s", fips, suffix,
            ROADMAP_DATA_TYPE );
	   roadmap_path_format (filename, sizeof (filename), map_path, name);
   }
   return filename;
}


/***********************************************************/
/*  Name        : check_sqlite_error_line()
 *  Purpose     : Auxiliary function. Checks error code
 *                  and prints sqlite message to the log inluding the line of the error statement
 *  Params		: [in] errstr - custom string
 *  			: [in] code - error code
 *  			: [in] line - line in the source code
 *				:
 */
static BOOL check_sqlite_error_line( const char* errstr, bool code, int line )
{

        if ( !code )
	{
            QByteArray errorBA;
            const char* errmsg = "";
            if (sgSQLiteDb)
            {
                errorBA = sgSQLiteDb->lastError().text().toLocal8Bit();
                errmsg = errorBA.data();
            }

            errstr = errstr ? errstr : "";
            roadmap_log( ROADMAP_MESSAGE_ERROR, __FILE__, line, "SQLite error in %s executing sqlite statement. Error : %d ( %s )",
                            errstr, code, errmsg );
            return FALSE;
	}
	return TRUE;
}

static const char* get_db_file( int fips )
{
   static char full_path[RM_TILE_STORAGE_DB_PATH_MAXSIZE] = {0};


   if ( !full_path[0] || ( fips != sgCurrentFips ) )
   {
#ifndef IPHONE_NATIVE
	  const char *map_path = roadmap_db_map_path ();
#else
      const char *map_path = roadmap_path_preferred("maps");
#endif //!IPHONE_NATIVE
      char db_name[RM_TILE_STORAGE_DB_NAME_SIZE];
      snprintf( db_name, RM_TILE_STORAGE_DB_NAME_SIZE, "%s%d%s", RM_TILE_STORAGE_DB_PREFIX, fips, RM_TILE_STORAGE_DB_SUFFIX );
      roadmap_path_format( full_path, sizeof( full_path ), map_path, db_name );
   }
   return full_path;
}
/***********************************************************/
/*  Name        : get_db()
 *  Purpose     : Auxiliary function. Opens the database file and returns the sqlite database handle
 *  Params		: [in] fips
 *  			:
 *				:
 */
static QSqlDatabase* get_db( int fips )
{
	const char* full_path = get_db_file( fips );     //  One time only for the current fips

	sgCurrentFips = fips;


	if ( ( sgConLifetime == _con_lifetime_application || sgIsInTransaction ) && sgSQLiteDb )
		return sgSQLiteDb;

	if ( !full_path[0] )
	{
		roadmap_log( ROADMAP_ERROR, "Can't define the database filename" );
		return sgSQLiteDb;
	}

        sgSQLiteDb = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
        sgSQLiteDb->setDatabaseName(full_path);

        check_sqlite_error( "opening database", sgSQLiteDb->open());

        check_sqlite_error( "pragma synchronous off", sgSQLiteDb->exec(RM_TILE_STORAGE_STMT_SYNC_OFF).lastError().type() == QSqlError::NoError);

        check_sqlite_error( "pragma count changes off", sgSQLiteDb->exec(RM_TILE_STORAGE_STMT_CNT_OFF).lastError().type() == QSqlError::NoError );

        check_sqlite_error( "pragma temp storage memory", sgSQLiteDb->exec(RM_TILE_STORAGE_STMT_TMP_STORE_MEM ).lastError().type() == QSqlError::NoError );

        check_sqlite_error( "pragma cache size", sgSQLiteDb->exec(RM_TILE_STORAGE_STMT_CACHE_SIZE).lastError().type() == QSqlError::NoError );

	if ( sgSQLiteDb != 0 && !sgTableExists )
        {
                check_sqlite_error( "pragma page size", sgSQLiteDb->exec(RM_TILE_STORAGE_STMT_PAGE_SIZE).lastError().type() == QSqlError::NoError );

                if ( check_sqlite_error( "creating table", sgSQLiteDb->exec(RM_TILE_STORAGE_STMT_CREATE_TABLE).lastError().type() == QSqlError::NoError) )
		{
			sgTableExists = TRUE;
		}
	}

        return sgSQLiteDb;
}

/***********************************************************/
/*  Name        : close_db()
 *  Purpose     : Auxiliary function. Closes the database
 *  Params		: void
 *  			:
 *				:
 */
static void close_db( void )
{
	if ( sgSQLiteDb )
	{
            sgSQLiteDb->close();
            delete sgSQLiteDb;
            sgSQLiteDb = NULL;
	}
}
/***********************************************************/
/*  Name        : trans_open( void )
 *  Purpose     : Auxiliary function. Opens the transactions. Sets the state to indicate that in transaction now
 *  Params		:
 *  			:
 *				:
 */
static QSqlDatabase*  trans_open( int fips )
{
        QSqlDatabase*  db = sgSQLiteDb;
	roadmap_log( ROADMAP_DEBUG, "Transaction open %d, %d ", sgIsInTransaction, sgTransStmtsCount );
	if ( sgIsInTransaction )
	{
		sgTransStmtsCount++;
		if ( sgTransStmtsCount >= RM_TILE_STORAGE_TRANS_STMTS_CNT )
		{
			roadmap_log( ROADMAP_DEBUG, "Transaction statements number exceeded - committing" );
			roadmap_main_remove_periodic( trans_timeout );
			trans_commit();
		}
		else
		{
			return db;
		}
	}

	db = get_db( fips );
	if ( !db )
	{
		roadmap_log( ROADMAP_ERROR, "Begin transaction failed - cannot open database" );
		return NULL;
	}
	roadmap_log( ROADMAP_DEBUG, "Transaction open %d, %d ", sgIsInTransaction, sgTransStmtsCount );

        if ( check_sqlite_error( "Begin transaction", db->exec("BEGIN TRANSACTION;").lastError().type() == QSqlError::NoError ) )
	{
		sgIsInTransaction = TRUE;
	}
	sgTransStmtsCount = 0;
	roadmap_main_set_periodic( RM_TILE_STORAGE_TRANS_TIMEOUT, trans_timeout );

	return db;
}

/***********************************************************/
/*  Name        : trans_commit( void )
 *  Purpose     : Auxiliary function. Commits the transaction to the DB
 *  Params		:
 *  			:
 *				:
 */
static void trans_commit( void )
{
	if ( !sgSQLiteDb )
	{
		roadmap_log( ROADMAP_ERROR, "Commit transaction failed - cannot open database" );
		return;
	}
        check_sqlite_error( "Commit transaction", sgSQLiteDb->exec("COMMIT;").lastError().type() == QSqlError::NoError );

	/*
	 * Close the database
	 */
	if ( sgConLifetime == _con_lifetime_session )
	{
		close_db();
	}

	sgIsInTransaction = FALSE;
	sgTransStmtsCount = 0;
}

/***********************************************************/
/*  Name        : trans_rollback( void )
 *  Purpose     : Auxiliary function. Rollbacks the transaction
 *  Params		:
 *  			:
 *				:
 */
static void trans_rollback( void )
{
	if ( !sgSQLiteDb )
	{
		roadmap_log( ROADMAP_ERROR, "Rollback transaction failed - cannot open database" );
		return;
	}

        check_sqlite_error( "Rollback transaction", sgSQLiteDb->exec("ROLLBACK;").lastError().type() == QSqlError::NoError );
	/*
	 * Close the database
	 */
	if ( sgConLifetime == _con_lifetime_session )
	{
		close_db();
	}
	sgIsInTransaction = FALSE;
	sgTransStmtsCount = 0;
}

/***********************************************************/
/*  Name        : trans_timeout( void )
 *  Purpose     : Auxiliary function. Commits the transaction on timer timeout
 *  Params		:
 *  			:
 *				:
 */
static void trans_timeout( void )
{
	roadmap_log( ROADMAP_DEBUG, "Transaction timeout expired - committing" );
	if ( sgIsInTransaction )
	{
		roadmap_main_remove_periodic( trans_timeout );
		trans_commit();
	}
}

/***********************************************************/
/*  Name        : roadmap_tile_store()
 *  Purpose     : Interface function. Stores the tile to the database
 *  Params		: [in] fips
 *  			: [in] tile_index - primary key
 *  			: [in] data - the pointer to the blob data
 *  			: [in] data - the size of the blob data block
 *				:
 */
int roadmap_tile_store (int fips, int tile_index, void *data, size_t size)
{
	int res = 0;
        QSqlDatabase* db = NULL;
	char stmt_string[RM_TILE_STORAGE_QUERY_MAXSIZE];

	// db = get_db( fips );
	db = trans_open( fips );

	if ( !db )
	{
		roadmap_log( ROADMAP_ERROR, "Tile storage failed - cannot open database" );
		return -1;
	}
	/*
	 * Prepare the string
	 */
	sprintf( stmt_string, RM_TILE_STORAGE_STMT_STORE, tile_index );

	/*
	 * Prepare the sqlite statement
	 */
        QSqlQuery query = db->exec();
        if ( !check_sqlite_error( "preparing the SQLITE statement", query.prepare(stmt_string) ) )
	{
		return -1;
	}
	/*
	 * Binding the data
         */
        QVariant dataVar(QByteArray((char*)data,size));
        query.bindValue(0, dataVar);
        if ( !check_sqlite_error( "binding the blob statement", query.lastError().type() == QSqlError::NoError ) )
	{
		return -1;
	}
	/*
	 * Evaluate
	 */
        if ( !check_sqlite_error( "finishing", query.exec() ) )
	{
		return -1;
	}

        query.finish();

	/*
	 * Close the database
	 */
	if ( sgConLifetime == _con_lifetime_session && !sgIsInTransaction )
	{
                db->close();
	}

	return res;
}


/***********************************************************/
/*  Name        : roadmap_tile_remove
 *  Purpose     : Interface function. Removes the tile from the database in one transaction
 *  Params		: [in] fips
 *  			: [in] tile_index - primary key
 *  			:
 *				:
 */
void roadmap_tile_remove (int fips, int tile_index)
{
        QSqlDatabase* db = NULL;

//	db = get_db( fips );
	db = trans_open( fips );

	if ( !db )
	{
		roadmap_log( ROADMAP_ERROR, "Tile remove failed - cannot open database" );
	}

	/*
	 * Prepare the sqlite statement
	 */
        QSqlQuery query = db->exec();
        if ( !check_sqlite_error( "preparing the SQLITE statement", query.prepare(RM_TILE_STORAGE_STMT_REMOVE) ) )
	{
		return;
	}
	/*
	 * Binding the parameter
	 */
        QVariant tileIndexVar(tile_index);
        query.bindValue(0, tileIndexVar);
        if ( !check_sqlite_error( "binding int parameter", query.lastError().type() == QSqlError::NoError ) )
	{
		return;
	}

        /*
         * Finalize
         */
        check_sqlite_error( "finishing", query.exec() );
        query.finish();

	/*
	 * Close the database
	 */
	if ( sgConLifetime == _con_lifetime_session  && !sgIsInTransaction )
	{
                db->close();
	}
}


static int roadmap_tile_file_load ( const char *full_name, void **base, size_t *size) {

   RoadMapFile		file;
   int				res;

   file = roadmap_file_open (full_name, "r");

   if (!ROADMAP_FILE_IS_VALID(file)) {
      return -1;
   }

#ifdef J2ME
   *size = favail(file);
#else
   *size = roadmap_file_length (NULL, full_name);
#endif
   *base = malloc (*size);

	   res = roadmap_file_read (file, *base, *size);
	   roadmap_file_close (file);

   if (res != (int)*size) {
      free (*base);
      return -1;
   }

   return 0;
}

/***********************************************************/
/*  Name        : roadmap_tile_load
 *  Purpose     : Interface function. Loads the tile data from the database.
 *                 Allocates the necessar heap space
 *  Params		: [in] fips
 *  			: [in] tile_index - primary key
 *  			: [out] base - the data storage address
 *				: [out] size - the size of the data block
 */
int roadmap_tile_load (int fips, int tile_index, void **data, size_t *size)
{
	int res = -1;
        QSqlDatabase* db = NULL;

	if ( tile_index == -1 )
	{
		const char* file_name = get_global_filename( fips );
                res = roadmap_tile_file_load( file_name, data, size );
		return res;
	}

//	db = get_db( fips );
	db = trans_open( fips );

	if ( !db )
	{
		roadmap_log( ROADMAP_ERROR, "Tile loading failed - cannot open database" );
		return -1;
	}

	/*
	 * Prepare the sqlite statement
	 */
        QSqlQuery query = db->exec();
        if ( !check_sqlite_error( "preparing the SQLITE statement", query.prepare(RM_TILE_STORAGE_STMT_LOAD) ) )
	{
		return -1;
	}
	/*
	 * Binding the parameter
         */
        QVariant tileIndexVar(tile_index);
        query.bindValue(0, tileIndexVar);
        if ( !check_sqlite_error( "binding int parameter", query.lastError().type() == QSqlError::NoError ) )
	{
		return -1;
	}

	/*
	 * Evaluate
         */
        bool found_results = true;
        if ( query.exec() && (found_results = query.next()))
	{
                QByteArray dataVar = query.value(0).toByteArray();
                *size = dataVar.length();
                *data = malloc( *size );
                roadmap_check_allocated( *data );
                memcpy( *data, dataVar.data(), *size );
		res = 0;
	}
	else
	{
		res = -1;
                check_sqlite_error( "select evaluation error", found_results );
	}

	/*
	 * Finalize
         */
        query.finish();

	/*
	 * Close the database
	 */
	if ( sgConLifetime == _con_lifetime_session && !sgIsInTransaction )
	{
                db->close();
	}

   return res;
}


/***********************************************************/
/*  Name        : roadmap_tile_remove_all
 *  Purpose     : Removes the entire database
 *
 *  Params     : [in] fips
 *             : [in] tile_index - primary key
 *             : [out] base - the data storage address
 *             : [out] size - the size of the data block
 */
void roadmap_tile_remove_all( int fips )
{
   const char *db_path = get_db_file( fips );

   // Close gracefully
   if ( sgIsInTransaction )
   {
      trans_rollback();
   }
   else
   {
      close_db();
   }


   // Reset state
   sgTableExists = FALSE;
   sgSQLiteDb = NULL;


   // Remove the db file
   roadmap_file_remove( db_path, NULL );
}



 
