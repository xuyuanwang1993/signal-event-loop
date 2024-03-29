/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2016  Intel Corporation. All rights reserved.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "gdbus/gdbus.h"
// #include "src/shared/util.h"
// #include "src/shared/shell.h"
#include "advertising.h"

#define AD_PATH "/org/bluez/advertising"
#define AD_IFACE "org.bluez.LEAdvertisement1"

struct ad_data {
	uint8_t data[25];
	uint8_t len;
};

struct service_data {
	char *uuid;
	struct ad_data data;
};

struct manufacturer_data {
	uint16_t id;
	struct ad_data data;
};

struct data {
	uint8_t type;
	struct ad_data data;
};

static struct ad {
	bool registered;
	char *type;
	char *local_name;
	char *secondary;
	uint16_t local_appearance;
	uint16_t duration;
	uint16_t timeout;
	uint16_t discoverable_to;
	char **uuids;
	size_t uuids_len;
	struct service_data service;
	struct manufacturer_data manufacturer;
	struct data data;
	bool discoverable;
	bool tx_power;
	bool name;
	bool appearance;
} ad = {
	.local_appearance = UINT16_MAX,
	.discoverable = true,
};

static void ad_release(DBusConnection *conn)
{
	ad.registered = false;

	g_dbus_unregister_interface(conn, AD_PATH, AD_IFACE);
}

static DBusMessage *release_advertising(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	printf("Advertising released\n");

	ad_release(conn);

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable ad_methods[] = {
	{ GDBUS_METHOD("Release", NULL, NULL, release_advertising) },
	{ }
};

static void register_setup(DBusMessageIter *iter, void *user_data)
{
	DBusMessageIter dict;
	const char *path = AD_PATH;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
				DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
				DBUS_TYPE_STRING_AS_STRING
				DBUS_TYPE_VARIANT_AS_STRING
				DBUS_DICT_ENTRY_END_CHAR_AS_STRING, &dict);
	dbus_message_iter_close_container(iter, &dict);
}

static void print_uuid(const char *uuid)
{
	return;
}

static void print_ad_uuids(void)
{
	char **uuid;

	for (uuid = ad.uuids; uuid && *uuid; uuid++)
		print_uuid(*uuid);
}

static void print_ad(void)
{
	print_ad_uuids();

	if (ad.service.uuid) {
		print_uuid(ad.service.uuid);
	}

	if (ad.manufacturer.data.len) {
		printf("Manufacturer: %u\n", ad.manufacturer.id);
	}

	if (ad.data.data.len) {
	}

	printf("Tx Power: %s\n", ad.tx_power ? "on" : "off");

	if (ad.local_name)
		printf("LocalName: %s\n", ad.local_name);
	else
		printf("Name: %s\n", ad.name ? "on" : "off");

	if (ad.local_appearance != UINT16_MAX)
		printf("Appearance: (0x%04x)\n",
					ad.local_appearance);
	else
		printf("Appearance: %s\n",
					ad.appearance ? "on" : "off");

	printf("Discoverable: %s\n", ad.discoverable ? "on": "off");

	if (ad.duration)
		printf("Duration: %u sec\n", ad.duration);

	if (ad.timeout)
		printf("Timeout: %u sec\n", ad.timeout);
}

static void register_reply(DBusMessage *message, void *user_data)
{
	DBusConnection *conn = user_data;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == FALSE) {
		ad.registered = true;
		printf("Advertising object registered\n");
		print_ad();
		/* Leave advertise running even on noninteractive mode */
	} else {
		printf("Failed to register advertisement: %s\n", error.name);
		dbus_error_free(&error);

		if (g_dbus_unregister_interface(conn, AD_PATH,
						AD_IFACE) == FALSE)
			printf("Failed to unregister advertising object\n");
	}
}

static gboolean get_type(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	const char *type = "peripheral";

	if (ad.type && strlen(ad.type) > 0)
		type = ad.type;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &type);

	return TRUE;
}

static gboolean uuids_exists(const GDBusPropertyTable *property, void *data)
{
	return ad.uuids_len != 0;
}

static gboolean get_uuids(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	DBusMessageIter array;
	size_t i;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "as", &array);

	for (i = 0; i < ad.uuids_len; i++)
		dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING,
							&ad.uuids[i]);

	dbus_message_iter_close_container(iter, &array);

	return TRUE;
}

static gboolean service_data_exists(const GDBusPropertyTable *property,
								void *data)
{
	return ad.service.uuid != NULL;
}

static gboolean get_service_data(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	DBusMessageIter dict;
	struct ad_data *data = &ad.service.data;
	uint8_t *val = data->data;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "{sv}", &dict);

	g_dbus_dict_append_array(&dict, ad.service.uuid, DBUS_TYPE_BYTE, &val,
								data->len);

	dbus_message_iter_close_container(iter, &dict);

	return TRUE;
}

static gboolean manufacturer_data_exists(const GDBusPropertyTable *property,
								void *data)
{
	return ad.manufacturer.id != 0;
}

static gboolean get_manufacturer_data(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	DBusMessageIter dict;
	struct ad_data *data = &ad.manufacturer.data;
	uint8_t *val = data->data;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "{qv}", &dict);

	g_dbus_dict_append_basic_array(&dict, DBUS_TYPE_UINT16,
					&ad.manufacturer.id,
					DBUS_TYPE_BYTE, &val, data->len);

	dbus_message_iter_close_container(iter, &dict);

	return TRUE;
}

static gboolean includes_exists(const GDBusPropertyTable *property, void *data)
{
	return ad.tx_power || ad.name || ad.appearance;
}

static gboolean get_includes(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	DBusMessageIter array;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "as", &array);

	if (ad.tx_power) {
		const char *str = "tx-power";

		dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);
	}

	if (ad.name) {
		const char *str = "local-name";

		dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);
	}

	if (ad.appearance) {
		const char *str = "appearance";

		dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);
	}

	dbus_message_iter_close_container(iter, &array);


	return TRUE;
}

static gboolean local_name_exits(const GDBusPropertyTable *property, void *data)
{
	return ad.local_name ? TRUE : FALSE;
}

static gboolean get_local_name(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &ad.local_name);

	return TRUE;
}

static gboolean appearance_exits(const GDBusPropertyTable *property, void *data)
{
	return ad.local_appearance != UINT16_MAX ? TRUE : FALSE;
}

static gboolean get_appearance(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16,
							&ad.local_appearance);

	return TRUE;
}

static gboolean duration_exits(const GDBusPropertyTable *property, void *data)
{
	return ad.duration;
}

static gboolean get_duration(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16, &ad.duration);

	return TRUE;
}

static gboolean timeout_exits(const GDBusPropertyTable *property, void *data)
{
	return ad.timeout;
}

static gboolean get_timeout(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16, &ad.timeout);

	return TRUE;
}

static gboolean data_exists(const GDBusPropertyTable *property, void *data)
{
	return ad.data.type != 0;
}

static gboolean get_data(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	DBusMessageIter dict;
	struct ad_data *data = &ad.data.data;
	uint8_t *val = data->data;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "{yv}", &dict);

	g_dbus_dict_append_basic_array(&dict, DBUS_TYPE_BYTE, &ad.data.type,
					DBUS_TYPE_BYTE, &val, data->len);

	dbus_message_iter_close_container(iter, &dict);

	return TRUE;
}

static gboolean discoverable_exists(const GDBusPropertyTable *property,
							void *data)
{
	return ad.discoverable;
}

static gboolean get_discoverable(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	dbus_bool_t value = ad.discoverable;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &value);

	return TRUE;
}

static gboolean discoverable_timeout_exits(const GDBusPropertyTable *property,
							void *data)
{
	return ad.discoverable_to;
}

static gboolean get_discoverable_timeout(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16,
							&ad.discoverable_to);

	return TRUE;
}

static gboolean secondary_exits(const GDBusPropertyTable *property, void *data)
{
	return ad.secondary ? TRUE : FALSE;
}

static gboolean get_secondary(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING,
							&ad.secondary);

	return TRUE;
}

static const GDBusPropertyTable ad_props[] = {
	{ "Type", "s", get_type },
	{ "ServiceUUIDs", "as", get_uuids, NULL, uuids_exists },
	{ "ServiceData", "a{sv}", get_service_data, NULL, service_data_exists },
	{ "ManufacturerData", "a{qv}", get_manufacturer_data, NULL,
						manufacturer_data_exists },
	{ "Data", "a{yv}", get_data, NULL, data_exists },
	{ "Discoverable", "b", get_discoverable, NULL, discoverable_exists },
	{ "DiscoverableTimeout", "q", get_discoverable_timeout, NULL,
						discoverable_timeout_exits },
	{ "Includes", "as", get_includes, NULL, includes_exists },
	{ "LocalName", "s", get_local_name, NULL, local_name_exits },
	{ "Appearance", "q", get_appearance, NULL, appearance_exits },
	{ "Duration", "q", get_duration, NULL, duration_exits },
	{ "Timeout", "q", get_timeout, NULL, timeout_exits },
	{ "SecondaryChannel", "s", get_secondary, NULL, secondary_exits },
	{ }
};

void ad_register(DBusConnection *conn, GDBusProxy *manager, const char *type)
{
	if (ad.registered) {
		printf("Advertisement is already registered\n");
	}

	g_free(ad.type);
	ad.type = g_strdup(type);

	if (g_dbus_register_interface(conn, AD_PATH, AD_IFACE, ad_methods,
					NULL, ad_props, NULL, NULL) == FALSE) {
		printf("Failed to register advertising object\n");
	}

	if (g_dbus_proxy_method_call(manager, "RegisterAdvertisement",
					register_setup, register_reply,
					conn, NULL) == FALSE) {
		printf("Failed to register advertising\n");
	}
}

static void unregister_setup(DBusMessageIter *iter, void *user_data)
{
	const char *path = AD_PATH;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
}

static void unregister_reply(DBusMessage *message, void *user_data)
{
	DBusConnection *conn = user_data;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == FALSE) {
		ad.registered = false;
		printf("Advertising object unregistered\n");
		if (g_dbus_unregister_interface(conn, AD_PATH,
							AD_IFACE) == FALSE)
			printf("Failed to unregister advertising"
					" object\n");
	} else {
		printf("Failed to unregister advertisement: %s\n",
								error.name);
		dbus_error_free(&error);
	}
}

void ad_unregister(DBusConnection *conn, GDBusProxy *manager)
{
	if (!manager)
		ad_release(conn);

	if (!ad.registered)
		return ;

	g_free(ad.type);
	ad.type = NULL;

	if (g_dbus_proxy_method_call(manager, "UnregisterAdvertisement",
					unregister_setup, unregister_reply,
					conn, NULL) == FALSE) {
		printf("Failed to unregister advertisement method\n");
	}
}

static void ad_clear_uuids(void)
{
	g_strfreev(ad.uuids);
	ad.uuids = NULL;
	ad.uuids_len = 0;
}

void ad_advertise_uuids(DBusConnection *conn, int argc, char *argv[])
{
	if (argc < 2 || !strlen(argv[1])) {
		print_ad_uuids();
		return ;
	}

	ad_clear_uuids();

	ad.uuids = g_strdupv(&argv[1]);
	if (!ad.uuids) {
		printf("Failed to parse input\n");
		return ;
	}

	ad.uuids_len = g_strv_length(ad.uuids);

	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE, "ServiceUUIDs");

	return ;
}

void ad_disable_uuids(DBusConnection *conn)
{
	if (!ad.uuids)
		return ;

	ad_clear_uuids();
	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE, "ServiceUUIDs");

	return ;
}

static void ad_clear_service(void)
{
	g_free(ad.service.uuid);
	memset(&ad.service, 0, sizeof(ad.service));

	return ;
}

static bool ad_add_data(struct ad_data *data, int argc, char *argv[])
{
	unsigned int i;

	memset(data, 0, sizeof(*data));

	for (i = 0; i < (unsigned int) argc; i++) {
		long int val;
		char *endptr = NULL;

		if (i >= G_N_ELEMENTS(data->data)) {
			printf("Too much data\n");
			return false;
		}

		val = strtol(argv[i], &endptr, 0);
		if (!endptr || *endptr != '\0' || val > UINT8_MAX) {
			printf("Invalid value at index %d\n", i);
			return false;
		}

		data->data[data->len] = val;
		data->len++;
	}

	return true;
}

void ad_advertise_service(DBusConnection *conn, int argc, char *argv[])
{
	struct ad_data data;

	if (argc < 2 || !strlen(argv[1])) {
		if (ad.service.uuid) {
			print_uuid(ad.service.uuid);
		}
		return ;
	}

	if (!ad_add_data(&data, argc - 2, argv + 2))
		return ;

	ad_clear_service();

	ad.service.uuid = g_strdup(argv[1]);
	ad.service.data = data;

	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE, "ServiceData");

	return ;
}

void ad_disable_service(DBusConnection *conn)
{
	if (!ad.service.uuid)
		return ;

	ad_clear_service();
	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE, "ServiceData");

	return ;
}

static void ad_clear_manufacturer(void)
{
	memset(&ad.manufacturer, 0, sizeof(ad.manufacturer));

	return ;
}

void ad_advertise_manufacturer(DBusConnection *conn, int argc, char *argv[])
{
	char *endptr = NULL;
	long int val;
	struct ad_data data;

	if (argc < 2 || !strlen(argv[1])) {
		if (ad.manufacturer.data.len) {
			printf("Manufacturer: %u\n",
						ad.manufacturer.id);
		}

		return ;
	}

	val = strtol(argv[1], &endptr, 0);
	if (!endptr || *endptr != '\0' || val > UINT16_MAX) {
		printf("Invalid manufacture id\n");
		return ;
	}

	if (!ad_add_data(&data, argc - 2, argv + 2))
		return ;

	ad_clear_manufacturer();
	ad.manufacturer.id = val;
	ad.manufacturer.data = data;

	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE,
							"ManufacturerData");

	return ;
}

void ad_disable_manufacturer(DBusConnection *conn)
{
	if (!ad.manufacturer.id && !ad.manufacturer.data.len)
		return ;

	ad_clear_manufacturer();
	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE,
							"ManufacturerData");

	return ;
}

static void ad_clear_data(void)
{
	memset(&ad.manufacturer, 0, sizeof(ad.manufacturer));

	return ;
}

void ad_advertise_data(DBusConnection *conn, int argc, char *argv[])
{
	char *endptr = NULL;
	long int val;
	struct ad_data data;

	if (argc < 2 || !strlen(argv[1])) {
		if (ad.manufacturer.data.len) {
			printf("Type: 0x%02x\n", ad.data.type);
		}

		return ;
	}

	val = strtol(argv[1], &endptr, 0);
	if (!endptr || *endptr != '\0' || val > UINT8_MAX) {
		printf("Invalid type\n");
		return ;
	}

	if (!ad_add_data(&data, argc - 2, argv + 2))
		return ;

	ad_clear_data();
	ad.data.type = val;
	ad.data.data = data;

	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE, "Data");

	return ;
}

void ad_disable_data(DBusConnection *conn)
{
	if (!ad.data.type && !ad.data.data.len)
		return ;

	ad_clear_data();
	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE, "Data");

	return ;
}

void ad_advertise_discoverable(DBusConnection *conn, dbus_bool_t *value)
{
	if (!value) {
		printf("Discoverable: %s\n",
				ad.discoverable ? "on" : "off");
		return ;
	}

	if (ad.discoverable == *value)
		return ;

	ad.discoverable = *value;

	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE, "Discoverable");

	return ;
}

void ad_advertise_discoverable_timeout(DBusConnection *conn, long int *value)
{
	if (!value) {
		if (ad.discoverable_to)
			printf("Timeout: %u sec\n",
					ad.discoverable_to);
		return ;
	}

	if (ad.discoverable_to == *value)
		return ;

	ad.discoverable_to = *value;

	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE,
					"DiscoverableTimeout");

	return ;
}

void ad_advertise_tx_power(DBusConnection *conn, dbus_bool_t *value)
{
	if (!value) {
		printf("Tx Power: %s\n", ad.tx_power ? "on" : "off");
		return ;
	}

	if (ad.tx_power == *value)
		return ;

	ad.tx_power = *value;

	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE, "Includes");

	return ;
}

void ad_advertise_name(DBusConnection *conn, bool value)
{
	if (ad.name == value)
		return ;

	ad.name = value;

	if (!value) {
		free(ad.local_name);
		ad.local_name = NULL;
	}

	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE, "Includes");

	return ;
}

void ad_advertise_local_name(DBusConnection *conn, const char *name)
{
	if (!name) {
		if (ad.local_name)
			printf("LocalName: %s\n", ad.local_name);
		else
			printf("Name: %s\n", ad.name ? "on" : "off");

		return ;
	}

	if (ad.local_name && !strcmp(name, ad.local_name))
		return;

	g_free(ad.local_name);
	ad.local_name = strdup(name);

	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE, "LocalName");

	return ;
}

void ad_advertise_appearance(DBusConnection *conn, bool value)
{
	if (ad.appearance == value)
		return ;

	ad.appearance = value;

	if (!value)
		ad.local_appearance = UINT16_MAX;

	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE, "Includes");

	return ;
}

void ad_advertise_local_appearance(DBusConnection *conn, long int *value)
{
	if (!value) {
		if (ad.local_appearance != UINT16_MAX)
			printf("Appearance: (0x%04x)\n",
					ad.local_appearance);
		else
			printf("Appearance: %s\n",
					ad.appearance ? "on" : "off");

		return ;
	}

	if (ad.local_appearance == *value)
		return ;

	ad.local_appearance = *value;

	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE, "Appearance");

	return ;
}

void ad_advertise_duration(DBusConnection *conn, long int *value)
{
	if (!value) {
		if (ad.duration)
			printf("Duration: %u sec\n", ad.duration);
		return ;
	}

	if (ad.duration == *value)
		return ;

	ad.duration = *value;

	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE, "Duration");

	return ;
}

void ad_advertise_timeout(DBusConnection *conn, long int *value)
{
	if (!value) {
		if (ad.timeout)
			printf("Timeout: %u sec\n", ad.timeout);
		return ;
	}

	if (ad.timeout == *value)
		return ;

	ad.timeout = *value;

	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE, "Timeout");

	return ;
}

void ad_advertise_secondary(DBusConnection *conn, const char *value)
{
	if (!value) {
		if (ad.secondary)
			 printf("Secondary Channel: %s\n",
							ad.secondary);
		return ;
	}

	if (ad.secondary && !strcmp(value, ad.secondary))
		return ;

	free(ad.secondary);

	if (value[0] == '\0') {
		ad.secondary = NULL;
		return ;
	}

	ad.secondary = strdup(value);

	g_dbus_emit_property_changed(conn, AD_PATH, AD_IFACE,
							"SecondaryChannel");

	return ;
}
