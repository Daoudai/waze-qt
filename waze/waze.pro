
include(qt/qt.pro)

# Add files and directories to ship with the application 
# by adapting the examples below.
# file1.source = myfile

datadir.source = assets/data

qmldir.source = qt/qml

DEPLOYMENTFOLDERS = datadir qmldir # file1 dir1

symbian:TARGET.UID3 = 0xE6919A30

# Smart Installer package's UID
# This UID is from the protected range 
# and therefore the package will fail to install if self-signed
# By default qmake uses the unprotected range value if unprotected UID is defined for the application
# and 0x2002CCCF value if protected UID is given to the application
#symbian:DEPLOYMENT.installer_header = 0x2002CCCF

# Allow network access on Symbian
symbian:TARGET.CAPABILITY += NetworkServices ReadUserData LocalServices NetworkServices UserEnvironment Location ReadDeviceData

# If your application uses the Qt Mobility libraries, uncomment
# the following lines and add the respective components to the 
# MOBILITY variable. 
# CONFIG += mobility
# MOBILITY +=

QT       += core gui sql network
TEMPLATE = app
TARGET = waze
CONFIG -= staticlib

linux {
    LIBS += -ldl -lrt -lssl -lcrypto
}

!maemo5 {
    QMAKE_CFLAGS += -Wno-unused-result -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-unused-variable
}

maemo5 | !isEmpty(MEEGO_VERSION_MAJOR) {
    DEFINES += HOME_PREFIX=\\\"MyDocs/.waze\\\"
}

!isEmpty(MEEGO_VERSION_MAJOR) {
# harmattan pulseaudio policy
    pulseaudio.files = waze_harmattan.conf
    pulseaudio.path = /etc/pulse/xpolicy.conf.d
    export(pulseaudio.files)
    export(pulseaudio.path)
    INSTALLS += pulseaudio
}



SOURCES += \
    roadmap_tile.c \
    roadmap_gzm.c \
    unix/roadmap_time.c \
    unix/roadmap_input_type.c \
    unix/roadmap_device_events.c \
    roadmap_dbread.c \
    roadmap_login.c \
    roadmap_welcome_wizard.c \
    roadmap_warning.c \
    roadmap_voice.c \
    roadmap_view.c \
    roadmap_utf8.c \
    roadmap_urlscheme.c \
    roadmap_turns.c \
    roadmap_tripserver.c \
    roadmap_trip.c \
    roadmap_tile_status.c \
    roadmap_ticker.c \
    roadmap_sunrise.c \
    roadmap_strings.c \
    roadmap_string.c \
    roadmap_street.c \
    roadmap_state.c \
    roadmap_start.c \
    roadmap_square.c \
    roadmap_sprite.c \
    roadmap_splash.c \
    roadmap_softkeys.c \
    roadmap_social.c \
    roadmap_skin.c \
    roadmap_shape.c \
    roadmap_search.c \
    roadmap_screen_obj.c \
    roadmap_scoreboard.c \
    roadmap_res_download.c \
    roadmap_res.c \
    roadmap_reminder.c \
    roadmap_range.c \
    roadmap_prompts.c \
    roadmap_preferences.c \
    roadmap_power.c \
    roadmap_polygon.c \
    roadmap_pointer.c \
    roadmap_point.c \
    roadmap_plugin.c \
    roadmap_phone_keyboard.c \
    roadmap_option.c \
    roadmap_object.c \
    roadmap_nmea.c \
    roadmap_net_mon.c \
    roadmap_navigate.c \
    roadmap_mood.c \
    roadmap_metadata.c \
    roadmap_message.c \
    roadmap_math.c \
    roadmap_map_settings.c \
    roadmap_map_download.c \
    roadmap_login_ssd.c \
    roadmap_locator.c \
    roadmap_line_speed.c \
    roadmap_line_route.c \
    roadmap_line.c \
    roadmap_layer.c \
    roadmap_label.c \
    roadmap_keyboard_text.c \
    roadmap_jpeg.c \
    roadmap_input.c \
    roadmap_httpcopy.c \
    roadmap_http_comp.c \
    roadmap_history.c \
    roadmap_help.c \
    roadmap_hash.c \
    roadmap_geo_location_info.c \
    roadmap_geo_config.c \
    roadmap_geocode.c \
    roadmap_general_settings.c \
    roadmap_fuzzy.c \
    roadmap_foursquare.c \
    roadmap_factory.c \
    roadmap_driver.c \
    roadmap_download_settings.c \
    roadmap_download.c \
    roadmap_display.c \
    roadmap_dictionary.c \
    roadmap_debug_info.c \
    roadmap_cyclic_array.c \
    roadmap_crossing.c \
    roadmap_county.c \
    roadmap_copy.c \
    roadmap_coord.c \
    roadmap_city_combobox.c \
    roadmap_city.c \
    roadmap_car.c \
    roadmap_camera_image.c \
    roadmap_browser.c \
    roadmap_border.c \
    roadmap_bar.c \
    roadmap_analytics.c \
    roadmap_alternative_routes.c \
    roadmap_alerter.c \
    roadmap_alert.c \
    roadmap_adjust.c \
    roadmap_address_tc.c \
    roadmap_address_ssd.c \
    roadmap.c \
    editor/editor_screen.c \
    editor/editor_points.c \
    editor/editor_plugin.c \
    editor/editor_main.c \
    editor/editor_cleanup.c \
    editor/editor_bar.c \
    editor/db/editor_trkseg.c \
    editor/db/editor_street.c \
    editor/db/editor_shape.c \
    editor/db/editor_point.c \
    editor/db/editor_override.c \
    editor/db/editor_marker.c \
    editor/db/editor_line.c \
    editor/db/editor_dictionary.c \
    editor/db/editor_db.c \
    editor/export/editor_upload.c \
    editor/export/editor_sync.c \
    editor/export/editor_report.c \
    editor/export/editor_download.c \
    editor/static/update_range.c \
    editor/static/notes.c \
    editor/static/editor_street_bar.c \
    editor/static/editor_dialog.c \
    editor/static/edit_marker.c \
    editor/static/add_alert.c \
    editor/track/editor_track_util.c \
    editor/track/editor_track_unknown.c \
    editor/track/editor_track_report.c \
    editor/track/editor_track_main.c \
    editor/track/editor_track_known.c \
    editor/track/editor_track_filter.c \
    editor/track/editor_track_compress.c \
    editor/track/editor_gps_data.c \
    md5.c \
    roadmap_tile_manager.c \
    roadmap_screen.c \
    ssd/ssd_dialog.c \
    ssd/ssd_widget_tab_order.c \
    ssd/ssd_widget.c \
    ssd/ssd_text.c \
    ssd/ssd_tabcontrol.c \
    ssd/ssd_separator.c \
    ssd/ssd_progress_msg_dialog.c \
    ssd/ssd_progress.c \
    ssd/ssd_popup.c \
    ssd/ssd_messagebox.c \
    ssd/ssd_menu.c \
    ssd/ssd_list.c \
    ssd/ssd_keyboard_layout.c \
    ssd/ssd_keyboard_dialog.c \
    ssd/ssd_keyboard.c \
    ssd/ssd_icon.c \
    ssd/ssd_generic_list_dialog.c \
    ssd/ssd_entry.c \
    ssd/ssd_contextmenu.c \
    ssd/ssd_container.c \
    ssd/ssd_confirm_dialog.c \
    ssd/ssd_combobox_dialog.c \
    ssd/ssd_combobox.c \
    ssd/ssd_choice.c \
    ssd/ssd_checkbox.c \
    ssd/ssd_button.c \
    ssd/ssd_bitmap.c  \
    Realtime/RealtimeUsers.c \
    Realtime/RealtimeTrafficInfoPlugin.c \
    Realtime/RealtimeTrafficInfo.c \
    Realtime/RealtimeSystemMessage.c \
    Realtime/RealtimePrivacy.c \
    Realtime/RealtimeOffline.c \
    Realtime/RealtimeNetRec.c \
    Realtime/RealtimeNetDefs.c \
    Realtime/RealtimeMath.c \
    Realtime/RealtimeDefs.c \
    Realtime/RealtimeBonus.c \
    Realtime/RealtimeAltRoutes.c \
    Realtime/RealtimeAlertsList.c \
    Realtime/RealtimeAlerts.c \
    Realtime/RealtimeAlertCommentsList.c \
    Realtime/Realtime.c \
    roadmap_log.c \
    roadmap_io.c \
    roadmap_list.c \
    roadmap_keyboard.c \
    roadmap_lang.c \
    address_search/local_search_dlg.c \
    address_search/local_search.c \
    address_search/generic_search_dlg.c \
    address_search/generic_search.c \
    address_search/address_search_dlg.c \
    address_search/address_search.c \
    websvc_trans/websvc_trans_queue.c \
    websvc_trans/websvc_trans.c \
    websvc_trans/websvc_address.c \
    websvc_trans/web_date_format.c \
    websvc_trans/string_parser.c \
    websvc_trans/socket_async_receive.c \
    websvc_trans/mkgmtime.c \
    websvc_trans/efficient_buffer.c \
    websvc_trans/cyclic_buffer.c \
    navigate/navigate_zoom.c \
    navigate/navigate_traffic.c \
    navigate/navigate_route_trans.c \
    navigate/navigate_route_astar.c \
    navigate/navigate_res_dlg.c \
    navigate/navigate_plugin.c \
    navigate/navigate_main.c \
    navigate/navigate_instr.c \
    navigate/navigate_graph.c \
    navigate/navigate_cost.c \
    navigate/fib-1.1/fib.c \
    roadmap_dialog.c \
    roadmap_device_array.c \
    roadmap_gpsd2.c \
    roadmap_gps.c \
    animation/roadmap_animation.c \
    ssd/ssd_entry_label.c \
    roadmap_social_image.c \
    roadmap_message_ticker.c \
    roadmap_groups.c \
    Realtime/RealtimeExternalPoiNotifier.c \
    Realtime/RealtimeExternalPoiDlg.c \
    Realtime/RealtimeExternalPoi.c \
    address_search/single_search_dlg.c \
    address_search/single_search.c \
    roadmap_groups_settings.c \
    tts/tts_voices.c \
    tts/tts_utils.c \
    tts/tts_ui.c \
    tts/tts_queue.c \
    tts/tts_db_files.c \
    tts/tts_db.c \
    tts/tts_cache.c \
    tts/tts.c \
    ssd/ssd_segmented_control.c \
    Realtime/RealtimeTrafficDetection.c \
    Realtime/RealtimePopUp.c \
    tts_was_provider.c \
    tts_apptext.c \
    roadmap_recorder_dlg.c \
    roadmap_recorder.c \
    navigate/navigate_tts.c \
    roadmap_base64.c \
    navigate/navigate_route_events.c \
    roadmap_zlib.c \
    zlib/zutil.c \
    zlib/uncompr.c \
    zlib/trees.c \
    zlib/inftrees.c \
    zlib/inflate.c \
    zlib/inffast.c \
    zlib/infback.c \
    zlib/gzio.c \
    zlib/deflate.c \
    zlib/crc32.c \
    zlib/compress.c \
    zlib/adler32.c

HEADERS += \
    roadmap_types.h \
    roadmap_tile.h \
    roadmap_path.h \
    roadmap_gzm.h \
    roadmap_file.h \
    roadmap.h \
    roadmap_data_format.h \
    Realtime/RealtimeUsers.h \
    Realtime/RealtimeTrafficInfoPlugin.h \
    Realtime/RealtimeTrafficInfo.h \
    Realtime/RealtimeSystemMessage.h \
    Realtime/RealtimePrivacy.h \
    Realtime/RealtimeOffline.h \
    Realtime/RealtimeNetDefs.h \
    Realtime/RealtimeNet.h \
    Realtime/RealtimeMath.h \
    Realtime/RealtimeDefs.h \
    Realtime/RealtimeBonus.h \
    Realtime/RealtimeAltRoutes.h \
    Realtime/RealtimeAlertsList.h \
    Realtime/RealtimeAlerts.h \
    Realtime/RealtimeAlertCommentsList.h \
    Realtime/Realtime.h \
    Realtime/LMap_Base.h \
    roadmap_messagebox.h \
    roadmap_dbread.h \
    roadmap_gps.h \
    roadmap_io.h \
    roadmap_net.h \
    roadmap_spawn.h \
    roadmap_list.h \
    roadmap_internet.h \
    roadmap_native_keyboard.h \
    roadmap_canvas.h \
    roadmap_gui.h \
    roadmap_keyboard.h \
    roadmap_input_type.h \
    roadmap_lang.h \
    roadmap_screen.h \
    address_search/local_search_dlg.h \
    address_search/local_search.h \
    address_search/generic_search_dlg.h \
    address_search/generic_search.h \
    address_search/address_search_dlg.h \
    address_search/address_search_defs.h \
    address_search/address_search.h \
    websvc_trans/websvc_trans_queue.h \
    websvc_trans/websvc_trans_defs.h \
    websvc_trans/websvc_trans.h \
    websvc_trans/websvc_address_defs.h \
    websvc_trans/websvc_address.h \
    websvc_trans/web_date_format_defs.h \
    websvc_trans/web_date_format.h \
    websvc_trans/string_parser.h \
    websvc_trans/socket_async_receive.h \
    websvc_trans/mkgmtime.h \
    websvc_trans/efficient_buffer.h \
    websvc_trans/cyclic_buffer.h \
    navigate/navigate_zoom.h \
    navigate/navigate_traffic.h \
    navigate/navigate_route_trans.h \
    navigate/navigate_route.h \
    navigate/navigate_res_dlg.h \
    navigate/navigate_plugin.h \
    navigate/navigate_main.h \
    navigate/navigate_instr.h \
    navigate/navigate_graph.h \
    navigate/navigate_cost.h \
    navigate/navigate_bar.h \
    navigate/fib-1.1/fibpriv.h \
    navigate/fib-1.1/fib.h \
    roadmap_login.h \
    roadmap_zlib.h \
    roadmap_welcome_wizard.h \
    roadmap_warning.h \
    roadmap_voice.h \
    roadmap_view.h \
    roadmap_utf8.h \
    roadmap_urlscheme.h \
    roadmap_turns.h \
    roadmap_tripserver.h \
    roadmap_trip.h \
    roadmap_trigonometry.h \
    roadmap_time.h \
    roadmap_tile_storage.h \
    roadmap_tile_status.h \
    roadmap_tile_model.h \
    roadmap_tile_manager.h \
    roadmap_ticker.h \
    roadmap_sunrise.h \
    roadmap_strings.h \
    roadmap_string.h \
    roadmap_street.h \
    roadmap_state.h \
    roadmap_start.h \
    roadmap_square.h \
    roadmap_sprite.h \
    roadmap_splash.h \
    roadmap_speedometer.h \
    roadmap_sound_stream.h \
    roadmap_sound.h \
    roadmap_softkeys.h \
    roadmap_social.h \
    roadmap_skin.h \
    roadmap_shape.h \
    roadmap_search.h \
    roadmap_screen_obj.h \
    roadmap_scoreboard.h \
    roadmap_res_download.h \
    roadmap_res.h \
    roadmap_reminder.h \
    roadmap_range.h \
    roadmap_prompts.h \
    roadmap_preferences.h \
    roadmap_power.h \
    roadmap_polygon.h \
    roadmap_pointer.h \
    roadmap_point.h \
    roadmap_plugin.h \
    roadmap_place.h \
    roadmap_phone_keyboard_defs.h \
    roadmap_phone_keyboard.h \
    roadmap_performance.h \
    roadmap_object.h \
    roadmap_nmea.h \
    roadmap_net_mon.h \
    roadmap_navigate.h \
    roadmap_mood.h \
    roadmap_metadata.h \
    roadmap_message.h \
    roadmap_math.h \
    roadmap_map_settings.h \
    roadmap_map_download.h \
    roadmap_main.h \
    roadmap_locator.h \
    roadmap_line_speed.h \
    roadmap_line_route.h \
    roadmap_line.h \
    roadmap_library.h \
    roadmap_layer.h \
    roadmap_label.h \
    roadmap_keyboard_text.h \
    roadmap_jpeg.h \
    roadmap_input.h \
    roadmap_httpcopy.h \
    roadmap_http_comp.h \
    roadmap_history.h \
    roadmap_help.h \
    roadmap_hash.h \
    roadmap_geo_location_info.h \
    roadmap_geo_config.h \
    roadmap_geocode.h \
    roadmap_general_settings.h \
    roadmap_fuzzy.h \
    roadmap_foursquare.h \
    roadmap_fileselection.h \
    roadmap_factory.h \
    roadmap_driver.h \
    roadmap_download_settings.h \
    roadmap_download.h \
    roadmap_display.h \
    roadmap_disclaimer.h \
    roadmap_dictionary.h \
    roadmap_dialog.h \
    roadmap_device_events.h \
    roadmap_device.h \
    roadmap_debug_info.h \
    roadmap_db_turns.h \
    roadmap_db_street.h \
    roadmap_db_square.h \
    roadmap_db_shape.h \
    roadmap_db_range.h \
    roadmap_db_polygon.h \
    roadmap_db_point.h \
    roadmap_db_place.h \
    roadmap_db_metadata.h \
    roadmap_db_line_speed.h \
    roadmap_db_line_route.h \
    roadmap_db_line.h \
    roadmap_db_dictionary.h \
    roadmap_db_county.h \
    roadmap_db_alert.h \
    roadmap_db.h \
    roadmap_cyclic_array_context.h \
    roadmap_cyclic_array.h \
    roadmap_crossing.h \
    roadmap_county_model.h \
    roadmap_county.h \
    roadmap_copyright.h \
    roadmap_copy.h \
    roadmap_coord.h \
    roadmap_config.h \
    roadmap_city_combobox.h \
    roadmap_city.h \
    roadmap_car.h \
    roadmap_camera_image.h \
    roadmap_camera_defs.h \
    roadmap_camera.h \
    roadmap_browser.h \
    roadmap_border.h \
    roadmap_bar.h \
    roadmap_analytics.h \
    roadmap_alternative_routes.h \
    roadmap_alerter.h \
    roadmap_alert.h \
    roadmap_adjust.h \
    roadmap_address_tc_defs.h \
    roadmap_address_tc.h \
    roadmap_address.h \
    editor/editor_screen.h \
    editor/editor_points.h \
    editor/editor_plugin.h \
    editor/editor_main.h \
    editor/editor_log.h \
    editor/editor_cleanup.h \
    editor/editor_bar.h \
    editor/db/editor_trkseg.h \
    editor/db/editor_street.h \
    editor/db/editor_square.h \
    editor/db/editor_shape.h \
    editor/db/editor_route.h \
    editor/db/editor_point.h \
    editor/db/editor_override.h \
    editor/db/editor_marker.h \
    editor/db/editor_line.h \
    editor/db/editor_dictionary.h \
    editor/db/editor_db.h \
    editor/export/editor_upload.h \
    editor/export/editor_sync.h \
    editor/export/editor_report.h \
    editor/export/editor_download.h \
    editor/static/update_range.h \
    editor/static/notes.h \
    editor/static/editor_street_bar.h \
    editor/static/editor_dialog.h \
    editor/static/edit_marker.h \
    editor/static/add_alert.h \
    editor/track/editor_track_util.h \
    editor/track/editor_track_unknown.h \
    editor/track/editor_track_report.h \
    editor/track/editor_track_main.h \
    editor/track/editor_track_known.h \
    editor/track/editor_track_filter.h \
    editor/track/editor_track_compress.h \
    editor/track/editor_gps_data.h \
    md5.h \
    ssd/ssd_dialog.h \
    ssd/ssd_widget.h \
    ssd/ssd_text.h \
    ssd/ssd_tabcontrol_defs.h \
    ssd/ssd_tabcontrol.h \
    ssd/ssd_separator.h \
    ssd/ssd_progress_msg_dialog.h \
    ssd/ssd_progress.h \
    ssd/ssd_popup.h \
    ssd/ssd_menu.h \
    ssd/ssd_list.h \
    ssd/ssd_keyboard_layout_defs.h \
    ssd/ssd_keyboard_layout.h \
    ssd/ssd_keyboard_dialog.h \
    ssd/ssd_keyboard.h \
    ssd/ssd_icon_defs.h \
    ssd/ssd_icon.h \
    ssd/ssd_generic_list_dialog.h \
    ssd/ssd_entry.h \
    ssd/ssd_contextmenu_defs.h \
    ssd/ssd_contextmenu.h \
    ssd/ssd_container.h \
    ssd/ssd_confirm_dialog.h \
    ssd/ssd_combobox_dialog.h \
    ssd/ssd_combobox.h \
    ssd/ssd_choice.h \
    ssd/ssd_checkbox.h \
    ssd/ssd_button.h \
    ssd/ssd_bitmap.h \
    ssd/keyboards_layout/WIDEGRID.h \
    ssd/keyboards_layout/QWERTY.h \
    ssd/keyboards_layout/GRID.h \
    roadmap_gpsd2.h \
    animation/roadmap_animation.h \
    ssd/ssd_entry_label.h \
    roadmap_social_image.h \
    roadmap_message_ticker.h \
    roadmap_groups.h \
    Realtime/RealtimeExternalPoiNotifier.h \
    Realtime/RealtimeExternalPoiDlg.h \
    Realtime/RealtimeExternalPoi.h \
    address_search/single_search_dlg.h \
    address_search/single_search.h \
    roadmap_groups_settings.h \
    tts/tts_voices.h \
    tts/tts_utils.h \
    tts/tts_ui.h \
    tts/tts_queue.h \
    tts/tts_provider.h \
    tts/tts_defs.h \
    tts/tts_db_sqlite.h \
    tts/tts_db_files.h \
    tts/tts_db.h \
    tts/tts_cache.h \
    tts/tts.h \
    ssd/ssd_segmented_control.h \
    Realtime/RealtimeTrafficDetection.h \
    Realtime/RealtimePopUp.h \
    tts_was_provider.h \
    tts_apptext_defs.h \
    tts_apptext.h \
    roadmap_tts.h \
    roadmap_recorder.h \
    navigate/navigate_tts_defs.h \
    navigate/navigate_tts.h \
    roadmap_base64.h \
    navigate/navigate_route_events.h \
    zlib/zutil.h \
    zlib/zlib.h \
    zlib/zconf.in.h \
    zlib/zconf.h \
    zlib/trees.h \
    zlib/inftrees.h \
    zlib/inflate.h \
    zlib/inffixed.h \
    zlib/inffast.h \
    zlib/deflate.h \
    zlib/crc32.h

OTHER_FILES += \
    qtc_packaging/debian_fremantle/README \
    qtc_packaging/debian_fremantle/changelog \
    qtc_packaging/debian_fremantle/compat \
    qtc_packaging/debian_fremantle/control \
    qtc_packaging/debian_fremantle/copyright \
    qtc_packaging/debian_fremantle/postinst \
    qtc_packaging/debian_fremantle/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/changelog \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/waze.aegis \
    qtc_packaging/debian_harmattan/postinst \
    qtc_packaging/debian_harmattan/rules \
    zlib/zlib.3 \
    zlib/README \
    zlib/Makefile.in \
    zlib/Makefile \
    zlib/INDEX \
    zlib/FAQ \
    zlib/configure \
    zlib/ChangeLog \
    zlib/algorithm.txt \
    qtc_packaging/debian_ubuntu_oneiric/rules \
    qtc_packaging/debian_ubuntu_oneiric/README \
    qtc_packaging/debian_ubuntu_oneiric/copyright \
    qtc_packaging/debian_ubuntu_oneiric/control \
    qtc_packaging/debian_ubuntu_oneiric/compat \
    qtc_packaging/debian_ubuntu_oneiric/changelog \
    qtc_packaging/debian_ubuntu_precise/rules \
    qtc_packaging/debian_ubuntu_precise/README \
    qtc_packaging/debian_ubuntu_precise/copyright \
    qtc_packaging/debian_ubuntu_precise/control \
    qtc_packaging/debian_ubuntu_precise/compat \
    qtc_packaging/debian_ubuntu_precise/changelog



# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()

#unix:!macx:!symbian: LIBS += -L$$PWD/qt/ -lwazeqt

INCLUDEPATH += $$PWD/qt
DEPENDPATH += $$PWD/qt

#unix:!macx:!symbian: PRE_TARGETDEPS += $$PWD/qt/libwazeqt.a


maemo5 {
    icon.files = waze.png
    icon.path = /usr/share/icons/hicolor/64x64/apps
    INSTALLS += icon
}









