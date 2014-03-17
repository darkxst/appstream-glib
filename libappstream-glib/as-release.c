/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2014 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "config.h"

#include <stdlib.h>

#include "as-node.h"
#include "as-release.h"
#include "as-tag.h"
#include "as-utils.h"

typedef struct _AsReleasePrivate	AsReleasePrivate;
struct _AsReleasePrivate
{
	gchar			*version;
	GHashTable		*descriptions;
	guint64			 timestamp;
};

G_DEFINE_TYPE_WITH_PRIVATE (AsRelease, as_release, G_TYPE_OBJECT)

#define GET_PRIVATE(o) (as_release_get_instance_private (o))

/**
 * as_release_finalize:
 **/
static void
as_release_finalize (GObject *object)
{
	AsRelease *release = AS_RELEASE (object);
	AsReleasePrivate *priv = GET_PRIVATE (release);

	g_free (priv->version);
	if (priv->descriptions != NULL)
		g_hash_table_unref (priv->descriptions);

	G_OBJECT_CLASS (as_release_parent_class)->finalize (object);
}

/**
 * as_release_init:
 **/
static void
as_release_init (AsRelease *release)
{
}

/**
 * as_release_class_init:
 **/
static void
as_release_class_init (AsReleaseClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = as_release_finalize;
}

/**
 * as_release_get_version:
 * @release: a #AsRelease instance.
 *
 * Gets the release version.
 *
 * Returns: string
 *
 * Since: 0.1.0
 **/
const gchar *
as_release_get_version (AsRelease *release)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	return priv->version;
}

/**
 * as_release_get_timestamp:
 * @release: a #AsRelease instance.
 *
 * Gets the release timestamp.
 *
 * Returns: timestamp
 *
 * Since: 0.1.0
 **/
guint64
as_release_get_timestamp (AsRelease *release)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	return priv->timestamp;
}

/**
 * as_release_get_description:
 * @release: a #AsRelease instance.
 * @locale: the locale, or %NULL. e.g. "en_GB"
 *
 * Gets the release description markup for a given locale.
 *
 * Returns: markup
 *
 * Since: 0.1.0
 **/
const gchar *
as_release_get_description (AsRelease *release, const gchar *locale)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	return as_hash_lookup_by_locale (priv->descriptions, locale);
}

/**
 * as_release_set_version:
 * @release: a #AsRelease instance.
 * @version: the version string.
 * @version_len: the size of @version, or -1 if %NULL-terminated.
 *
 * Sets the release version.
 *
 * Since: 0.1.0
 **/
void
as_release_set_version (AsRelease *release,
			const gchar *version,
			gssize version_len)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	priv->version = as_strndup (version, version_len);
}

/**
 * as_release_set_timestamp:
 * @release: a #AsRelease instance.
 * @timestamp: the timestamp value.
 *
 * Sets the release timestamp.
 *
 * Since: 0.1.0
 **/
void
as_release_set_timestamp (AsRelease *release, guint64 timestamp)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	priv->timestamp = timestamp;
}

/**
 * as_release_set_description:
 * @release: a #AsRelease instance.
 * @locale: the locale, or %NULL. e.g. "en_GB"
 * @description: the description markup.
 * @description_len: the size of @description, or -1 if %NULL-terminated.
 *
 * Sets the description release markup.
 *
 * Since: 0.1.0
 **/
void
as_release_set_description (AsRelease *release,
			    const gchar *locale,
			    const gchar *description,
			    gssize description_len)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	if (locale == NULL)
		locale = "C";
	if (priv->descriptions == NULL) {
		priv->descriptions = g_hash_table_new_full (g_str_hash,
							    g_str_equal,
							    g_free,
							    g_free);
	}
	g_hash_table_insert (priv->descriptions,
			     g_strdup (locale),
			     as_strndup (description, description_len));
}

/**
 * as_release_node_insert: (skip)
 * @release: a #AsRelease instance.
 * @parent: the parent #GNode to use..
 *
 * Inserts the release into the DOM tree.
 *
 * Returns: (transfer full): A populated #GNode
 *
 * Since: 0.1.0
 **/
GNode *
as_release_node_insert (AsRelease *release, GNode *parent)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	GNode *n;
	gchar *timestamp_str;

	timestamp_str = g_strdup_printf ("%" G_GUINT64_FORMAT,
					 priv->timestamp);
	n = as_node_insert (parent, "release", NULL,
			    AS_NODE_INSERT_FLAG_NONE,
			    "timestamp", timestamp_str,
			    "version", priv->version,
			    NULL);
	if (priv->descriptions != NULL) {
		as_node_insert_localized (n, "description", priv->descriptions,
					  AS_NODE_INSERT_FLAG_PRE_ESCAPED);
	}
	g_free (timestamp_str);
	return n;
}

/**
 * as_release_node_parse:
 * @release: a #AsRelease instance.
 * @node: a #GNode.
 * @error: A #GError or %NULL.
 *
 * Populates the object from a DOM node.
 *
 * Returns: %TRUE for success
 *
 * Since: 0.1.0
 **/
gboolean
as_release_node_parse (AsRelease *release, GNode *node, GError **error)
{
	const gchar *tmp;
	GNode *n;
	GString *xml;

	tmp = as_node_get_attribute (node, "timestamp");
	if (tmp != NULL)
		as_release_set_timestamp (release, atol (tmp));
	tmp = as_node_get_attribute (node, "version");
	if (tmp != NULL)
		as_release_set_version (release, tmp, -1);

	/* descriptions are translated and optional */
	for (n = node->children; n != NULL; n = n->next) {
		if (as_tag_from_string (as_node_get_name (n)) != AS_TAG_DESCRIPTION)
			continue;
		xml = as_node_to_xml (n->children, AS_NODE_TO_XML_FLAG_NONE);
		as_release_set_description (release,
					    as_node_get_attribute (n, "xml:lang"),
					    xml->str, xml->len);
		g_string_free (xml, TRUE);
	}
	return TRUE;
}

/**
 * as_release_new:
 *
 * Creates a new #AsRelease.
 *
 * Returns: (transfer full): a #AsRelease
 *
 * Since: 0.1.0
 **/
AsRelease *
as_release_new (void)
{
	AsRelease *release;
	release = g_object_new (AS_TYPE_RELEASE, NULL);
	return AS_RELEASE (release);
}
