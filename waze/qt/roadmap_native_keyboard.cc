/* roadmap_native_keyboard.cc - native keyboard support
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

extern "C" {
    #include "roadmap_native_keyboard.h"
}
#include <QSystemDeviceInfo>
QTM_USE_NAMESPACE

BOOL roadmap_native_keyboard_enabled( void ) {
#ifndef Q_WS_MAEMO_5
    QSystemDeviceInfo info;
    return (info.inputMethodType() & QSystemDeviceInfo::Keyboard) &&
            (info.keyboardTypes() & QSystemDeviceInfo::FullQwertyKeyboard) &&
            info.isKeyboardFlippedOpen();
#else
    /* TODO */ //maemo5
    return false;
#endif
}

BOOL roadmap_native_keyboard_visible( void ) {
    /* TODO */
    return true;
}

void roadmap_native_keyboard_show( RMNativeKBParams* params ) {
    /* TODO */
//    android_kb_action_type type;

//    if ( !roadmap_native_keyboard_enabled() )
//            return;
/*
    switch ( params->action_button )
    {
            case _native_kb_action_default:
            {
                    type = _andr_ime_action_none;
                    break;
            }
            case _native_kb_action_done:
            {
                    type = _andr_ime_action_done;
            }	break;
            case _native_kb_action_search:
            {
                    type = _andr_ime_action_search;
                    break;
            }
            case _native_kb_action_next:
            {
                    type = _andr_ime_action_next;
                    break;
            }

            default:
            {
                    type = _andr_ime_action_none;
                    break;
            }
    } // switch

    FreeMapNativeManager_ShowSoftKeyboard( type, params->close_on_action ); */
}

void roadmap_native_keyboard_hide( void ) {
    /* TODO */
}

void roadmap_native_keyboard_get_params( RMNativeKBParams* params_out ) {
    /* TODO */
}
