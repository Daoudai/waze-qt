
/* tts_db_sqlite.c - implementation of the Text To Speech (TTS) database layer
 *                       SQlite based
 *
 * LICENSE:
 *   Copyright 2011, Waze Ltd      Alex Agranovich (AGA)
 *   Copyright 2011, Waze Ltd      Assaf Paz
 *
 *
 *   This file is part of RoadMap.
 *
 *   RoadMap is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License V2 as published by
 *   the Free Software Foundation.
 *
 *   RoadMap is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with RoadMap; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * SYNOPSYS:
 *
 *   See tts_db_sqlite.h
 *       tts_db.h
 *       tts.h
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <QtSql>

extern "C" {
#include "roadmap.h"
#include "roadmap_main.h"
#include "roadmap_performance.h"
#include "roadmap_path.h"
#include "roadmap_file.h"
#include "../tts/tts_db_sqlite.h"
}

//======================== Local defines ========================

#define   TTS_DB_SQLITE_QUERY_MAXSIZE        1024
#define   TTS_DB_SQLITE_PATH_MAXSIZE      512
#define   TTS_DB_SQLITE_TRANS_TIMEOUT        2500L       // Transaction timeout in msec
#define   TTS_DB_SQLITE_TRANS_STMTS_CNT      200         // Transaction timeout in msec

#define   TTS_DB_SQLITE_FILE_NAME            "tts.db"

#define   TTS_DB_SQLITE_STMT_CREATE_TABLE   "CREATE TABLE IF NOT EXISTS '%s' (text TEXT, data BLOB, path TEXT, storage_type INTEGER, text_type INTEGER, PRIMARY KEY(text))"
#define   TTS_DB_SQLITE_STMT_STORE          "INSERT OR REPLACE INTO '%s' values (?,?,?,?,?);"
#define   TTS_DB_SQLITE_STMT_LOAD           "SELECT data, storage_type, path FROM '%s' WHERE text=?;"
#define   TTS_DB_SQLITE_STMT_LOAD_INFO      "SELECT storage_type, path, text_type FROM '%s' WHERE text=?;"
#define   TTS_DB_SQLITE_STMT_REMOVE         "DELETE FROM '%s' WHERE text=?;"
#define   TTS_DB_SQLITE_STMT_DROP_TABLE     "DROP TABLE IF EXISTS '%s';"
#define   TTS_DB_SQLITE_STMT_EXISTS         "SELECT EXISTS ( SELECT 1 FROM '%s' WHERE text=?);"
#define   TTS_DB_SQLITE_STMT_SYNC_OFF       "PRAGMA synchronous = OFF"
#define   TTS_DB_SQLITE_STMT_CNT_OFF        "PRAGMA count_changes = OFF"
#define   TTS_DB_SQLITE_STMT_TMP_STORE_MEM  "PRAGMA temp_store = MEMORY"
#define   TTS_DB_SQLITE_STMT_CACHE_SIZE     "PRAGMA cache_size = 2000"
#define   TTS_DB_SQLITE_STMT_PAGE_SIZE      "PRAGMA page_size = 8192"
#define   TTS_DB_SQLITE_STMT_ENCODING       "PRAGMA encoding = 'UTF-8'"

#define check_sqlite_error( errstr, code ) \
   _check_sqlite_error_line( errstr, code, __LINE__ )

//======================== Local types ========================

typedef enum
{
   _con_lifetime_session = 0x0,     // Connection remains active only within one statement execution
   _con_lifetime_application       // Connection remains active within application
} RMTtsStorageConLifetime;

//======================== Globals ========================

static QSqlDatabase* sgSQLiteDb  = NULL;       // The current db handle
static RMTtsStorageConLifetime sgConLifetime = _con_lifetime_session;     // The type of connection lifetime

static BOOL sgIsInTransaction = FALSE;
static int  sgTransStmtsCount = 0;

//======================== Local Declarations ========================

static QSqlDatabase*  _trans_open( const char* table_name );
static void _trans_commit( void );
static void _trans_rollback( void );
static void _trans_timeout( void );
static BOOL _check_sqlite_error_line( const char* errstr, int code, int line );
static QSqlDatabase* _get_db( void );
static void _close_db( void );
static const char* _get_db_file( void );
static const char* _table_name( const char* _name );

/*
 ******************************************************************************
 */

BOOL tts_db_sqlite_store( const TtsDbEntry* entry, TtsDbDataStorageType storage_type, const TtsData* db_data, const TtsPath* db_path )
{
   QSqlDatabase* db = NULL;
   char stmt_string[TTS_DB_SQLITE_QUERY_MAXSIZE];
   const char* path = db_path ? db_path->path : "";

   db = _trans_open( entry->voice_id );

   if ( !db )
   {
      roadmap_log( ROADMAP_ERROR, "TTS storage failed - cannot open database" );
      return FALSE;
   }
   /*
    * Prepare the string
    */
   sprintf( stmt_string, TTS_DB_SQLITE_STMT_STORE, _table_name( entry->voice_id ) );

   roadmap_log( ROADMAP_DEBUG, "TTS SQLite Store. Query string: %s", stmt_string );

   /*
    * Prepare the sqlite statement
    */
   QSqlQuery query = db->exec();
   if ( !check_sqlite_error( "preparing the SQLITE statement", query.prepare(stmt_string) ) )
   {
        return FALSE;
   }

   /*
    * Binding the data
    */
   QVariant text(QString(entry->text));
   query.bindValue(0, text);
   if ( !check_sqlite_error( "binding the text statement", query.lastError().type() == QSqlError::NoError ) )
   {
      return FALSE;
   }
   if ( db_data && db_data->data )
   {
      QVariant dataVar(QByteArray((char*)db_data->data, db_data->data_size));
      query.bindValue(1, dataVar);
      if ( !check_sqlite_error( "binding the blob statement", query.lastError().type() == QSqlError::NoError ) )
      {
         return FALSE;
      }
   }
   if ( db_path )
   {
       QVariant pathVar(QString(path));
       query.bindValue(2, pathVar);
      if ( !check_sqlite_error( "binding the path statement", query.lastError().type() == QSqlError::NoError ) )
      {
         return FALSE;
      }
   }

   QVariant storageType(storage_type);
   query.bindValue(3, storageType);
   if ( !check_sqlite_error( "binding the storage type statement", query.lastError().type() == QSqlError::NoError ) )
   {
      return FALSE;
   }
   QVariant textType(entry->text_type);
   query.bindValue(4, textType);
   if ( !check_sqlite_error( "binding the text type statement", query.lastError().type() == QSqlError::NoError ) )
   {
      return FALSE;
   }
   /*
    * Evaluate
    */
   if ( !check_sqlite_error( "finishing", query.exec() ) )
   {
      return FALSE;
   }

   query.finish();

   /*
    * Close the database
    */
   if ( sgConLifetime == _con_lifetime_session && !sgIsInTransaction )
   {
        db->close();
   }

   return TRUE;
}

/*
 ******************************************************************************
 */
BOOL tts_db_sqlite_remove( const TtsDbEntry* entry )
{
   QSqlDatabase* db = NULL;
   char stmt_string[TTS_DB_SQLITE_QUERY_MAXSIZE];

   db = _trans_open( NULL );

   if ( !db )
   {
      roadmap_log( ROADMAP_ERROR, "TTS cache remove failed - cannot open database" );
   }

   /*
    * Prepare the string
    */
   sprintf( stmt_string, TTS_DB_SQLITE_STMT_REMOVE, _table_name( entry->voice_id ) );

   /*
    * Prepare the sqlite statement
    */
   QSqlQuery query = db->exec();
   if ( !check_sqlite_error( "preparing the SQLITE statement", query.prepare(stmt_string) ) )
   {
      return FALSE;
   }

   /*
    * Binding the data
    */
   QVariant text(QString(entry->text));
   query.bindValue(0, text);
   if ( !check_sqlite_error( "binding the text statement", query.lastError().type() == QSqlError::NoError ) )
   {
      return FALSE;
   }

   /*
    * Evaluate
    */
   check_sqlite_error( "statement evaluation", query.exec() );

   /*
    * Finalize
    */
   query.finish();

   /*
    * Close the database
    */
   if ( sgConLifetime == _con_lifetime_session  && !sgIsInTransaction )
   {
     db->close();
   }
   return TRUE;
}

/*
 ******************************************************************************
 */
BOOL tts_db_sqlite_get_info( const TtsDbEntry* entry, TtsTextType* text_type, TtsDbDataStorageType* storage_type, TtsPath* db_path )
{
   int res = TRUE;
   QSqlDatabase* db = NULL;
   char stmt_string[TTS_DB_SQLITE_QUERY_MAXSIZE];

   db = _trans_open( NULL );

   if ( !db )
   {
      roadmap_log( ROADMAP_ERROR, "Tile loading failed - cannot open database" );
      return FALSE;
   }

   /*
    * Prepare the string
    */
   sprintf( stmt_string, TTS_DB_SQLITE_STMT_LOAD_INFO, _table_name( entry->voice_id ) );

   /*
    * Prepare the sqlite statement
    */
   QSqlQuery query = db->exec();
   if ( !check_sqlite_error( "preparing the SQLITE statement", query.prepare(stmt_string) ) )
   {
      return FALSE;
   }

   /*
    * Binding the data
    */
   QVariant text(QString(entry->text));
   query.bindValue(0, text);
   if ( !check_sqlite_error( "binding the text statement", query.lastError().type() == QSqlError::NoError ) )
   {
      return FALSE;
   }

   /*
    * Evaluate
    */
   if ( query.exec() && query.next())
   {
      // Storage type
      if ( storage_type )
        *storage_type = (TtsDbDataStorageType) query.value(0).toInt();
      // Path data
      QString pathVar = query.value(1).toString();
      if ( db_path )
      {
         if ( !pathVar.isNull() )
         {
            strncpy_safe( db_path->path, pathVar.toAscii().data(), TTS_PATH_MAXLEN );
            res = TRUE;
         }
         else
         {
            db_path->path[0] = 0;
         }
      }
      // Text type
      if ( text_type )
         *text_type = (TtsTextType) query.value(2).toInt();
   }
   else
   {
      res = FALSE;
      check_sqlite_error( "select evaluation", query.lastError().type() == QSqlError::NoError );
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

/*
 ******************************************************************************
 */
BOOL tts_db_sqlite_get( const TtsDbEntry* entry, TtsDbDataStorageType* storage_type, TtsData *db_data, TtsPath* db_path )
{
   int res = TRUE;
   QSqlDatabase* db = NULL;
   char stmt_string[TTS_DB_SQLITE_QUERY_MAXSIZE];

   db = _trans_open( NULL );

   if ( !db )
   {
      roadmap_log( ROADMAP_ERROR, "Tile loading failed - cannot open database" );
      return FALSE;
   }

   /*
    * Prepare the string
    */
   sprintf( stmt_string, TTS_DB_SQLITE_STMT_LOAD, _table_name( entry->voice_id ) );

   /*
    * Prepare the sqlite statement
    */
   QSqlQuery query = db->exec();
   if ( !check_sqlite_error( "preparing the SQLITE statement", query.prepare(stmt_string) ) )
   {
      return FALSE;
   }

   /*
    * Binding the data
    */
   QVariant text(QString(entry->text));
   query.bindValue(0, text);
   if ( !check_sqlite_error( "binding the text statement", query.lastError().type() == QSqlError::NoError ) )
   {
      return FALSE;
   }

   /*
    * Evaluate
    */
   if ( query.exec() && query.next())
   {
      const char* path_data;
      QByteArray dataVar = query.value(0).toByteArray();
      if ( !dataVar.isNull() && ( dataVar.length() > 0 ) )
      {
          db_data->data_size = dataVar.length();
          db_data->data = malloc( db_data->data_size );
          roadmap_check_allocated( db_data->data );
          memcpy( db_data->data, dataVar.data(), db_data->data_size );
      }

      if ( storage_type )
      {
         *storage_type = (TtsDbDataStorageType) query.value(1).toInt();
      }

      if ( db_path )
      {
         path_data = query.value(1).toString().toAscii().data();
         if ( path_data )
         {
            strncpy_safe( db_path->path, path_data, sizeof( db_path->path ) );
            res = TRUE;
         }
         else
         {
            db_path->path[0] = 0;
         }
      }
   }
   else
   {
      res = FALSE;
      check_sqlite_error( "select evaluation", query.lastError().type() == QSqlError::NoError );
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

/*
 ******************************************************************************
 */
BOOL tts_db_sqlite_exists( const TtsDbEntry* entry )
{
   int res = TRUE;
   QSqlDatabase* db = NULL;
   char stmt_string[TTS_DB_SQLITE_QUERY_MAXSIZE];

   db = _trans_open( entry->voice_id );

   if ( !db )
   {
      roadmap_log( ROADMAP_ERROR, "Tile loading failed - cannot open database" );
      return FALSE;
   }

   /*
    * Prepare the string
    */
   sprintf( stmt_string, TTS_DB_SQLITE_STMT_EXISTS, ( entry->voice_id ) );

   /*
    * Prepare the sqlite statement
    */
   QSqlQuery query = db->exec();
   if ( !check_sqlite_error( "preparing the SQLITE statement", query.prepare(stmt_string) ) )
   {
      return FALSE;
   }

   /*
    * Binding the data
    */
   QVariant text(QString(entry->text));
   query.bindValue(0, text);
   if ( !check_sqlite_error( "binding the text statement", query.lastError().type() == QSqlError::NoError ) )
   {
      return FALSE;
   }

   /*
    * Evaluate
    */
   if ( query.exec() && query.next())
   {
      res = query.value(0).toInt();
   }
   else
   {
      res = FALSE;
      check_sqlite_error( "select evaluation", query.lastError().type() == QSqlError::NoError );
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
/*
 ******************************************************************************
 */
BOOL tts_db_sqlite_destroy_voice( const char* voice_id )
{
   QSqlDatabase* db = NULL;
   char stmt_string[TTS_DB_SQLITE_QUERY_MAXSIZE];

   db = _trans_open( NULL );

   if ( !db )
   {
      roadmap_log( ROADMAP_ERROR, "TTS cache remove failed - cannot open database" );
   }

   /*
    * Prepare the string
    */
   sprintf( stmt_string, TTS_DB_SQLITE_STMT_DROP_TABLE, _table_name( voice_id ) );

   roadmap_log( ROADMAP_DEBUG, "TTS SQLite Destroy voice. Query string: %s. Voice: %s", stmt_string, SAFE_STR( voice_id ) );

   /*
    * Prepare the sqlite statement
    */
   QSqlQuery query = db->exec();
   if ( !check_sqlite_error( "preparing the SQLITE statement", query.prepare(stmt_string) ) )
   {
      return FALSE;
   }

   /*
    * Evaluate
    */
   check_sqlite_error( "statement evaluation", query.exec() );

   /*
    * Finalize
    */
   query.finish();

   /*
    * Explicitly commit drop table statement
    */
   _trans_commit();

   /*
    * Close the database
    */
   if ( sgConLifetime == _con_lifetime_session  && !sgIsInTransaction )
   {
      db->close();
   }

   return TRUE;
}

/*
 ******************************************************************************
 */
void tts_db_sqlite_destroy( void )
{
   const char *db_path = _get_db_file();

   // Close gracefully
   if ( sgIsInTransaction )
   {
      _trans_rollback();
   }
   else
   {
      _close_db();
   }

   // Reset state
   sgSQLiteDb = NULL;

   // Remove the db file
   roadmap_file_remove( db_path, NULL );
}

/***********************************************************/
/*  Name        : _check_sqlite_error_line()
 *  Purpose     : Auxiliary function. Checks error code
 *                  and prints sqlite message to the log inluding the line of the error statement
 *  Params     : [in] errstr - custom string
 *          : [in] code - error code
 *          : [in] line - line in the source code
 *          :
 */
static BOOL _check_sqlite_error_line( const char* errstr, int code, int line )
{

   if ( !code )
   {
       QByteArray errorBA;
       const char* errmsg = "";
       int errorCode = 0;
       if (sgSQLiteDb)
       {
           errorBA = sgSQLiteDb->lastError().text().toLocal8Bit();
           errmsg = errorBA.data();
           errorCode = sgSQLiteDb->lastError().number();
       }

      errstr = errstr ? errstr : "";
      roadmap_log( ROADMAP_MESSAGE_ERROR, __FILE__, line, "SQLite error in %s executing sqlite statement. Error : %d ( %s )",
            errstr, errorCode, errmsg );
      return FALSE;
   }
   return TRUE;
}

static const char* _get_db_file( void )
{
   static char full_path[TTS_DB_SQLITE_PATH_MAXSIZE] = {0};

   if ( !full_path[0] )
   {
      const char *path = roadmap_path_tts();
      roadmap_path_format( full_path, sizeof( full_path ), path, TTS_DB_SQLITE_FILE_NAME );
   }
   return full_path;
}



/***********************************************************/
/*  Name        : _get_db()
 *  Purpose     : Auxiliary function. Opens the database file and returns the sqlite database handle
 *  Params     : [in] void
 *          :
 *          :
 */
static QSqlDatabase* _get_db( void )
{

   char* error_msg;
   const char* full_path = _get_db_file();     //  One time only for the current fips

   if ( ( sgConLifetime == _con_lifetime_application || sgIsInTransaction ) && sgSQLiteDb )
      return sgSQLiteDb;

   if ( !full_path[0] )
   {
      roadmap_log( ROADMAP_ERROR, "Can't define the database filename" );
      return sgSQLiteDb;
   }

   sgSQLiteDb = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
   sgSQLiteDb->setDatabaseName(full_path);

   check_sqlite_error( "opening database", sgSQLiteDb->open() );

   check_sqlite_error( "pragma synchronous off", sgSQLiteDb->exec(TTS_DB_SQLITE_STMT_SYNC_OFF).lastError().type() == QSqlError::NoError);

   check_sqlite_error( "pragma count changes off", sgSQLiteDb->exec(TTS_DB_SQLITE_STMT_CNT_OFF).lastError().type() == QSqlError::NoError);

   check_sqlite_error( "pragma temp storage memory", sgSQLiteDb->exec(TTS_DB_SQLITE_STMT_TMP_STORE_MEM).lastError().type() == QSqlError::NoError);

   check_sqlite_error( "pragma cache size", sgSQLiteDb->exec(TTS_DB_SQLITE_STMT_CACHE_SIZE).lastError().type() == QSqlError::NoError);

   check_sqlite_error( "pragma encoding", sgSQLiteDb->exec(TTS_DB_SQLITE_STMT_ENCODING).lastError().type() == QSqlError::NoError);

   if ( sgSQLiteDb != NULL )
   {
      check_sqlite_error( "pragma page size", sgSQLiteDb->exec(TTS_DB_SQLITE_STMT_PAGE_SIZE).lastError().type() == QSqlError::NoError);
   }

   return sgSQLiteDb;
}

/***********************************************************/
/*  Name        : _close_db()
 *  Purpose     : Auxiliary function. Closes the database
 *  Params     : void
 *          :
 *          :
 */
static void _close_db( void )
{
   if ( sgSQLiteDb )
   {
       sgSQLiteDb->close();
       delete sgSQLiteDb;
       sgSQLiteDb = NULL;
   }
}
/***********************************************************/
/*  Name        : _trans_open( void )
 *  Purpose     : Auxiliary function. Opens the transactions. Sets the state to indicate that in transaction now
 *  Params      : [in] table - table to work on
 *              :
 *              :
 */
static QSqlDatabase*  _trans_open( const char* table_name )
{
   QSqlDatabase*  db = sgSQLiteDb;
   roadmap_log( ROADMAP_DEBUG, "Transaction open %d, %d ", sgIsInTransaction, sgTransStmtsCount );
   if ( sgIsInTransaction )
   {
      sgTransStmtsCount++;
      if ( sgTransStmtsCount >= TTS_DB_SQLITE_TRANS_STMTS_CNT )
      {
         roadmap_log( ROADMAP_DEBUG, "Transaction statements number exceeded - committing" );
         roadmap_main_remove_periodic( _trans_timeout );
         _trans_commit();
      }
      else
      {
         return db;
      }
   }

   db = _get_db();
   if ( db != NULL  )
   {
      if ( table_name != NULL )
      {
         char create_table_query[TTS_DB_SQLITE_QUERY_MAXSIZE];

         // Creating the table if not exists
         sprintf( create_table_query, TTS_DB_SQLITE_STMT_CREATE_TABLE, table_name );
         check_sqlite_error( "creating table", db->exec(create_table_query).lastError().type() == QSqlError::NoError );
      }

      roadmap_log( ROADMAP_DEBUG, "Transaction open %d, %d ", sgIsInTransaction, sgTransStmtsCount );

      if ( check_sqlite_error( "Begin transaction", db->exec( "BEGIN TRANSACTION;").lastError().type() == QSqlError::NoError ) )
      {
         sgIsInTransaction = TRUE;
      }
      sgTransStmtsCount = 0;
      roadmap_main_set_periodic( TTS_DB_SQLITE_TRANS_TIMEOUT, _trans_timeout );
   }
   else
   {
      roadmap_log( ROADMAP_ERROR, "Begin transaction failed - cannot open database" );
   }

   return db;
}

/***********************************************************/
/*  Name        : _trans_commit( void )
 *  Purpose     : Auxiliary function. Commits the transaction to the DB
 *  Params     :
 *          :
 *          :
 */
static void _trans_commit( void )
{

//   int ret_val;

   if ( !sgSQLiteDb )
   {
      roadmap_log( ROADMAP_ERROR, "Commit transaction failed - cannot open database" );
      return;
   }
   check_sqlite_error( "Commit transaction", sgSQLiteDb->exec("COMMIT;").lastError().type() == QSqlError::NoError );

   roadmap_log( ROADMAP_DEBUG, "Transaction committed %d, %d", sgIsInTransaction, sgTransStmtsCount );

   /*
    * Close the database
    */
   if ( sgConLifetime == _con_lifetime_session )
   {
      _close_db();
   }

   sgIsInTransaction = FALSE;
   sgTransStmtsCount = 0;
}

/***********************************************************/
/*  Name        : _trans_rollback( void )
 *  Purpose     : Auxiliary function. Rollbacks the transaction
 *  Params     :
 *          :
 */
static void _trans_rollback( void )
{

//   int ret_val;

   if ( !sgSQLiteDb )
   {
      roadmap_log( ROADMAP_ERROR, "Rollback transaction failed - cannot open database" );
      return;
   }

   check_sqlite_error( "Rollback transaction", sgSQLiteDb->exec("ROLLBACK;").lastError().type() == QSqlError::NoError);
   /*
    * Close the database
    */
   if ( sgConLifetime == _con_lifetime_session )
   {
      _close_db();
   }
   sgIsInTransaction = FALSE;
   sgTransStmtsCount = 0;
}

/***********************************************************/
/*  Name       : _trans_timeout( void )
 *  Purpose    : Auxiliary function. Commits the transaction on timer timeout
 *  Params     :
 *             :
  */
static void _trans_timeout( void )
{
   roadmap_log( ROADMAP_DEBUG, "Transaction timeout expired - committing" );
   if ( sgIsInTransaction )
   {
      roadmap_main_remove_periodic( _trans_timeout );
      _trans_commit();
   }
}
/***********************************************************/
/*  Name       : _table_name( void )
 *  Purpose    : Table name parser. Builds valid table names from the requested ones
 *  Params     :
 *             :
  */
static const char* _table_name( const char* _name )
{
   static char s_table_name[TTS_DB_SQLITE_QUERY_MAXSIZE];

   char* pCh = &s_table_name[0];

   strncpy_safe( s_table_name, _name, TTS_DB_SQLITE_QUERY_MAXSIZE );

//   for( ; *pCh; pCh++ )
//   {
//      if ( *pCh == '-')
//         *pCh = '_';
//   }
   return s_table_name;
}

