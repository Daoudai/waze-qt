/* roadmap_camera.h - The interface for the camera related functionality
 *
 * LICENSE:
 *
 *   Copyright 2009, Waze Ltd
 *                   Alex Agranovich
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
 */

#ifndef INCLUDE__ROADMAP_CAMERA__H
#define INCLUDE__ROADMAP_CAMERA__H
#ifdef __cplusplus
extern "C" {
#endif

#include "roadmap.h"
#include "roadmap_camera_defs.h"

// 1. Takes camera capture and saves the image according to the attributes in image_file
// 2. Creates and returns thumbnail buffer according to the attributes in image_thumbnail.
//    The pointer to the buffer is passed back in the image_thumbnail ( see roadmap_camera_defs.h
BOOL roadmap_camera_take_picture( CameraImageFile* image_file, CameraImageBuf* image_thumbnail );

#ifdef __cplusplus
}
#endif
#endif /* INCLUDE__ROADMAP_CAMERA__H */

