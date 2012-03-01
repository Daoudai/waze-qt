/* roadmap_zlib.cc - Compress / Decompress files
 *
 * LICENSE:
 *
 *   Copyright 2012 Assaf Paz
 *
 *   This file is part of RoadMap.
 *
 *   RoadMap is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
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
 *   See roadmap_zlib.h
 */

#include <QByteArray>
#include <QFile>
#include <QDataStream>

extern "C" {
#include "roadmap_zlib.h"
}

///////////////////////////////////////////////////////
// Compress (gzip) in_file to out_file
int roadmap_zlib_compress (const char* in_path, const char* in_file, const char* out_path, const char* out_file, int level, BOOL isLogFile) {

    QFile infile(QString().fromLocal8Bit(in_path).append("/").append(in_file));
    QFile outfile(QString().fromLocal8Bit(out_path).append("/").append(out_file));

    infile.open(QIODevice::ReadOnly);
    outfile.open(QIODevice::WriteOnly);

    QByteArray uncompressedData = infile.readAll();
    QByteArray compressedData = qCompress(uncompressedData/*.left(2).right(4)*/, level);

    outfile.write(compressedData);

    infile.close();
    outfile.close();

    return 0;
}


int roadmap_zlib_uncompress(void *raw_data, unsigned long *raw_data_size, void *compressed_data, int compressed_data_size) {

    QByteArray inData((char*) compressed_data, compressed_data_size);
    QByteArray outData = qUncompress(inData);
    *raw_data_size = outData.length();
    raw_data = new char*[*raw_data_size];

    memcpy(raw_data, outData.data(), *raw_data_size);

    return 0;
}

///////////////////////////////////////////////////////
// Decompress using zlib
// not implemented

