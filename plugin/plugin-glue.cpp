/*
 * plugin-glue.cpp: MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "moonlight.h"
#include "plugin.h"
#include "plugin-class.h"
#include "moon-mono.h"

NPError 
NPP_New (NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved)
{
	DEBUGMSG ("NPP_New");

	if (!instance)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = new PluginInstance (instance, mode);
	if (plugin == NULL)
		return NPERR_OUT_OF_MEMORY_ERROR;

	plugin->Initialize (argc, argn, argv);
	instance->pdata = plugin;

	return NPERR_NO_ERROR;
}

NPError 
NPP_Destroy (NPP instance, NPSavedData** save)
{
	DEBUGMSG ("NPP_Destroy, instance=%p\n", instance);

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	plugin->Finalize ();

	delete plugin;
	instance->pdata = NULL;

	return NPERR_NO_ERROR;
}

NPError 
NPP_SetWindow (NPP instance, NPWindow* window)
{
	DEBUGMSG ("NPP_SetWindow %d %d", window->width, window->height);

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->SetWindow (window);
}

NPError
NPP_NewStream (NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype)
{
	DEBUGMSG ("NPP_NewStream");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->NewStream (type, stream, seekable, stype);
}

NPError
NPP_DestroyStream (NPP instance, NPStream* stream, NPError reason)
{
	DEBUGMSG ("NPP_DestroyStream");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->DestroyStream (stream, reason);
}

void
NPP_StreamAsFile (NPP instance, NPStream* stream, const char* fname)
{
	DEBUGMSG ("NPP_StreamAsFile");

	if (instance == NULL)
		return;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	plugin->StreamAsFile (stream, fname);
}

int32
NPP_WriteReady (NPP instance, NPStream* stream)
{
	DEBUGMSG ("NPP_WriteReady");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->WriteReady (stream);
}

int32
NPP_Write (NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer)
{
	DEBUGMSG ("NPP_Write");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->Write (stream, offset, len, buffer);
}

void
NPP_Print (NPP instance, NPPrint* platformPrint)
{
	DEBUGMSG ("NPP_Print");

	if (instance == NULL)
		return;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	plugin->Print (platformPrint);
}

void
NPP_URLNotify (NPP instance, const char* url, NPReason reason, void* notifyData)
{
	DEBUGMSG ("NPP_URLNotify");

	if (instance == NULL)
		return;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	plugin->UrlNotify (url, reason, notifyData);
}


int16 
NPP_HandleEvent (NPP instance, void* event)
{
	DEBUGMSG ("NPP_HandleEvent");

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->EventHandle (event);
}

NPError 
NPP_GetValue (NPP instance, NPPVariable variable, void *result)
{
	DEBUGMSG ("NPP_GetValue %d (%x)", variable, variable);

	NPError err = NPERR_NO_ERROR;

	switch (variable) {
		case NPPVpluginNameString:
			*((char **)result) = PLUGIN_NAME;
			break;
		case NPPVpluginDescriptionString:
			*((char **)result) = PLUGIN_DESCRIPTION;
			break;
		case NPPVpluginNeedsXEmbed:
			*((PRBool *)result) = PR_TRUE;
			break;
		default:
			if (instance == NULL)
				return NPERR_INVALID_INSTANCE_ERROR;

			PluginInstance *plugin = (PluginInstance *) instance->pdata;
			err = plugin->GetValue (variable, result);
	}

	return err;
}

NPError 
NPP_SetValue (NPP instance, NPNVariable variable, void *value)
{
	DEBUGMSG ("NPP_SetValue %d (%p)", variable, value);

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	PluginInstance *plugin = (PluginInstance *) instance->pdata;
	return plugin->SetValue (variable, value);
}

char *
NPP_GetMIMEDescription (void)
{
	DEBUGMSG ("NPP_GetMIMEDescription");
	return (MIME_TYPES_HANDLED);
}

void downloader_initialize ();

static bool already_initialized = false;

NPError
NPP_Initialize (void)
{
	DEBUGMSG ("NP_Initialize");

	// We dont need to initialize mono vm and gtk more than one time.
	if (!already_initialized) {
		already_initialized = true;
		gtk_init (0, 0);
		downloader_initialize ();
		vm_init ();
		runtime_init ();

		plugin_init_classes ();
	}
	TimeManager::Instance()->Start();

	return NPERR_NO_ERROR;
}

void
NPP_Shutdown (void)
{
	DEBUGMSG ("NP_Shutdown");
	
	// runtime_shutdown is broken at moment so let us just shutdown TimeManager,
	// when fixed please uncomment above line and remove time manger shutdown.
	//runtime_shutdown ();
	TimeManager::Instance()->Shutdown ();
}
