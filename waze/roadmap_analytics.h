/* roadmap_analytics.h
 *
 * LICENSE:
 *
 *   Copyright 2010 Avi R.
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
 */


#ifndef ROADMAP_ANALYTICS_H_
#define ROADMAP_ANALYTICS_H_

void roadmap_analytics_log_event (const char *event_name, const char *info_name, const char *info_val);

#endif //ROADMAP_ANALYTICS_H_