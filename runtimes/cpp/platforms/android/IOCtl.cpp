/* Copyright (C) 2010 MoSync AB

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.
*/

#include "IOCtl.h"

#include <helpers/cpp_defs.h>

/**
 * Custom Wide Char String length calculation function.
 * Exists because the android NDK does not support wchars.
 *
 * @param s		input multi-byte string
 *
 * @return		length of the input string
 *
 * TODO: Use a build in function instead if this one.
 * This function is written in a non-readable way,
 * improved it a little bit by changing a variable name.
 */
inline size_t wideCharStringLength(const wchar * s)
{
	const wchar *current;
	if (s == 0)
	{
		return 0;
	}
	for (current = s; *current; ++current);
	return current - s;
}

namespace Base
{
	int _maFrameBufferGetInfo(MAFrameBufferInfo *info)
	{
		int size = maGetScrSize();
		int width = (size&0xffff0000) >> 16;
		int height = size&0x0000ffff;

		info->bitsPerPixel = 32;
		info->bytesPerPixel = 4;
		info->redMask = 0x000000ff;
		info->greenMask = 0x0000ff00;
		info->blueMask = 0x00ff0000;


		info->width = width;
		info->height = height;
		info->pitch = info->width*4;

		info->sizeInBytes = info->pitch * info->height;

		info->redShift = 0;
		info->greenShift = 8;
		info->blueShift = 16;

		info->redBits = 8;
		info->greenBits = 8;
		info->blueBits = 8;

		info->supportsGfxSyscalls = 0;

		return 1;
	}


	int _maFrameBufferInit(void *data, int memStart, JNIEnv* jNIEnv, jobject jThis)
	{
		int rdata = (int)data - memStart;

		//char* b = (char*)malloc(200);
		//sprintf(b,"Framebuffer data: %i", rdata);
		//__android_log_write(ANDROID_LOG_INFO,"JNI",b);
		//free(b);

		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "_enableFramebuffer", "(I)V");
		if (methodID == 0) return 0;
		jNIEnv->CallVoidMethod(jThis, methodID, rdata);

		jNIEnv->DeleteLocalRef(cls);

		return 1;
	}

	int _maFrameBufferClose(JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "_disableFramebuffer", "()V");
		if (methodID == 0) return 0;
		jNIEnv->CallVoidMethod(jThis, methodID);

		jNIEnv->DeleteLocalRef(cls);

		return 1;
	}

	int _maBtStartDeviceDiscovery(int names, JNIEnv* jNIEnv, jobject jThis)
	{
		//__android_log_write(ANDROID_LOG_INFO, "JNI Syscalls", "_maBtStartDeviceDiscovery begin");

		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maBtStartDeviceDiscovery", "(I)I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID, names);

		jNIEnv->DeleteLocalRef(cls);

		//__android_log_write(ANDROID_LOG_INFO, "JNI Syscalls", "_maBtStartDeviceDiscovery end");

		return (int)ret;
	}

	/**
	 * Returns actual length of device name.
	 */
	int _maBtGetNewDevice(
		int memStart,
		int nameBufPointer,
		int nameBufSize,
		int actualNameLengthPointer,
		int addressPointer,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		//__android_log_write(ANDROID_LOG_INFO, "JNI Syscalls", "_maBtGetNewDevice begin");

		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maBtGetNewDevice", "(IIII)I");
		if (methodID == 0) return 0;

		jint ret = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			nameBufPointer - memStart,
			nameBufSize,
			actualNameLengthPointer - memStart,
			addressPointer - memStart);

		jNIEnv->DeleteLocalRef(cls);

		//__android_log_write(ANDROID_LOG_INFO, "JNI Syscalls", "_maBtGetNewDevice end");

		return (int)ret;
	}

	int _maBtStartServiceDiscovery(MABtAddr* addr, MAUUID* uuid, JNIEnv* jNIEnv, jobject jThis)
	{
		//__android_log_write(ANDROID_LOG_INFO, "JNI Syscalls", "_maBtStartServiceDiscovery begin");

		// Device address converted to string.
		char addressBuf[64];
		sprintf(addressBuf, "%02X%02X%02X%02X%02X%02X",
			addr->a[0], addr->a[1], addr->a[2], addr->a[3], addr->a[4], addr->a[5]);
		jstring jstrAddress = jNIEnv->NewStringUTF(addressBuf);

		// UUID converted to string.
		char uuidBuf[64];
		char* u = (char*) uuid;
		sprintf(uuidBuf, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
			u[3], u[2], u[1], u[0], u[7], u[6], u[5], u[4], u[11], u[10], u[9], u[8], u[15], u[14], u[13], u[12]);
		jstring jstrUUID = jNIEnv->NewStringUTF(uuidBuf);

		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maBtStartServiceDiscovery", "(Ljava/lang/String;Ljava/lang/String;)I");
		if (methodID == 0) return 0;

		jint ret = jNIEnv->CallIntMethod(jThis, methodID, jstrAddress, jstrUUID);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrAddress);
		jNIEnv->DeleteLocalRef(jstrUUID);

		//__android_log_write(ANDROID_LOG_INFO, "JNI Syscalls", "_maBtStartServiceDiscovery end");

		return (int)ret;
	}

	int _maBtGetNextServiceSize(
		int memStart,
		int nameBufSizePointer,
		int nUuidsPointer,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		//__android_log_write(ANDROID_LOG_INFO, "JNI Syscalls", "_maBtGetNextServiceSize begin");

		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maBtGetNextServiceSize", "(II)I");
		if (methodID == 0) return 0;

		jint ret = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			nameBufSizePointer - memStart,
			nUuidsPointer - memStart);

		jNIEnv->DeleteLocalRef(cls);

		//__android_log_write(ANDROID_LOG_INFO, "JNI Syscalls", "_maBtGetNextServiceSize end");

		return (int)ret;
	}

	int _maBtGetNewService(
		int memStart,
		int portPointer,
		int nameBufPointer,
		int nameBufSize,
		int uuidsPointer,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		//__android_log_write(ANDROID_LOG_INFO, "JNI Syscalls", "_maBtGetNewService begin");

		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maBtGetNewService", "(IIII)I");
		if (methodID == 0) return 0;

		jint ret = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			portPointer - memStart,
			nameBufPointer - memStart,
			nameBufSize,
			uuidsPointer - memStart);

		jNIEnv->DeleteLocalRef(cls);

		//__android_log_write(ANDROID_LOG_INFO, "JNI Syscalls", "_maBtGetNewService end");

		return (int)ret;
	}

	int _maBtCancelDiscovery(JNIEnv* jNIEnv, jobject jThis)
	{
		//__android_log_write(ANDROID_LOG_INFO, "JNI Syscalls", "_maBtCancelDiscovery begin");

		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maBtCancelDiscovery", "()I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID);
		jNIEnv->DeleteLocalRef(cls);

		//__android_log_write(ANDROID_LOG_INFO, "JNI Syscalls", "_maBtCancelDiscovery end");

		return (int)ret;
	}

	int _maAccept(int serverHandle, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maAccept", "(I)I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID, serverHandle);
		jNIEnv->DeleteLocalRef(cls);
		return (int)ret;
	}

	int _maLocationStart(JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maLocationStart", "()I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID);
		jNIEnv->DeleteLocalRef(cls);

		return (int)ret;
	}

	int _maLocationStop(JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maLocationStop", "()I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID);
		jNIEnv->DeleteLocalRef(cls);

		return (int)ret;
	}

	int _maSendTextSMS(
		const char* phoneNo,
		const char* message,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		// Get Java objects.
		jstring jstringPhoneNo = jNIEnv->NewStringUTF(phoneNo);
		jstring jstringMessage = jNIEnv->NewStringUTF(message);
		jclass theClass = jNIEnv->GetObjectClass(jThis);

		// Get the method.
		jmethodID methodID = jNIEnv->GetMethodID(
			theClass,
			"maSendTextSMS",
			"(Ljava/lang/String;Ljava/lang/String;)I");

		// Check that the method exists.
		if (0 == methodID )
		{
			return -1;
		}

		// Call the method
		jint result = jNIEnv->CallIntMethod(
			jThis, methodID, jstringPhoneNo, jstringMessage);

		// Release Java objects.
		jNIEnv->DeleteLocalRef(jstringPhoneNo);
		jNIEnv->DeleteLocalRef(jstringMessage);
		jNIEnv->DeleteLocalRef(theClass);

		return (int)result;
	}

	int _maGetSystemProperty(
		const char* key,
		int buf,
		int memStart,
		int size,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jstring jstrKey = jNIEnv->NewStringUTF(key);

		int rBuf = buf - memStart;

		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls, "maGetSystemProperty", "(Ljava/lang/String;II)I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID, jstrKey, rBuf, size);
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrKey);

		return (int)ret;
	}

	int _maPlatformRequest(const char* url, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrURL = jNIEnv->NewStringUTF(url);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPlatformRequest", "(Ljava/lang/String;)I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID, jstrURL);
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrURL);

		return (int)ret;
	}

	int _maWriteLog(const char* str, int b, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrLOG = jNIEnv->NewStringUTF(str);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maWriteLog", "(Ljava/lang/String;I)I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID, jstrLOG);
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrLOG);

		return (int)ret;
	}

	/**
	 * Internal function corresponding to the maShowVirtualKeyboard IOCtl.
	 * Shows the android soft keyboard.
	 *
	 * @param jNIEnv	JNI environment used
	 * @param jThis		Pointer to the java class
	 *
	 * @return			Value returned by the maTextBox
	 *					java method
	 */
	int _maShowVirtualKeyboard(JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID =
			jNIEnv->GetMethodID(cls, "maShowVirtualKeyboard", "()I");
		if (methodID == 0) return 0;
		int ret = jNIEnv->CallIntMethod(jThis, methodID);
		jNIEnv->DeleteLocalRef(cls);

		return ret;
	}

	/**
	 * Internal function corresponding to the maTextBox IOCtl.
	 * Displays a full screen editable text field with
	 * OK and Cancel buttons.
	 *
	 * @param title			Title of the text box
	 * @param inText		Initial content of the text box
	 * @param outText		Buffer that will contain the text
	 *						entered by the user
	 * @param maxSize		Maximum size of outText
	 * @param constraints	Not implemented yet
	 * @param memStart		Pointer to the begining of the
	 *						MoSync memory
	 * @param jNIEnv		JNI environment used
	 * @param jThis			Pointer to the java class
	 *
	 * @return				Value returned by the maTextBox
	 *						java method
	 */
	int _maTextBox(
		const wchar* title,
		const wchar* inText,
		int outText,
		int maxSize,
		int constraints,
		int memStart,
		JNIEnv* jNIEnv,
		jobject jThis)
	{

		// Initialization.
		jstring jstrTITLE = jNIEnv->NewString((jchar*)title, wideCharStringLength(title));
		jstring jstrINTEXT = jNIEnv->NewString((jchar*)inText, wideCharStringLength(inText));

		jclass cls = jNIEnv->GetObjectClass(jThis);

		// Remove the offset from the output buffer's address
		int rBuf = outText - memStart;

		// Call the java method
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maTextBox",
												 "(Ljava/lang/String;Ljava/lang/String;III)I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID, jstrTITLE,
										 jstrINTEXT, rBuf, maxSize, constraints);

		// Clean
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrTITLE);
		jNIEnv->DeleteLocalRef(jstrINTEXT);

		return (int)ret;
	}

	int _maMessageBox(const char* title, const char* text, JNIEnv* jNIEnv, jobject jThis)
	{
		Base::gSyscall->VM_Yield();

		jstring jstrTitle = jNIEnv->NewStringUTF(title);
		jstring jstrText = jNIEnv->NewStringUTF(text);

		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(cls, "maMessageBox", "(Ljava/lang/String;Ljava/lang/String;)I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID, jstrTitle, jstrText);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrTitle);
		jNIEnv->DeleteLocalRef(jstrText);

		return ret;
	}

	int _maAlert(const char* title, const char* message, const char* button1,
					const char* button2, const char* button3, JNIEnv* jNIEnv, jobject jThis)
	{
		Base::gSyscall->VM_Yield();

		jstring jstrTitle = jNIEnv->NewStringUTF(title);
		jstring jstrText = jNIEnv->NewStringUTF(message);
		jstring jstrBtn1 = jNIEnv->NewStringUTF(button1);
		jstring jstrBtn2 = jNIEnv->NewStringUTF(button2);
		jstring jstrBtn3 = jNIEnv->NewStringUTF(button3);

		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(cls, "maAlert", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID, jstrTitle, jstrText, jstrBtn1, jstrBtn2, jstrBtn3);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrTitle);
		jNIEnv->DeleteLocalRef(jstrText);
		jNIEnv->DeleteLocalRef(jstrBtn1);
		jNIEnv->DeleteLocalRef(jstrBtn2);
		jNIEnv->DeleteLocalRef(jstrBtn3);

		return ret;
	}

	int _maImagePickerOpen(JNIEnv* jNIEnv, jobject jThis)
	{
		Base::gSyscall->VM_Yield();

		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(cls, "maImagePickerOpen", "()I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID);

		jNIEnv->DeleteLocalRef(cls);

		return ret;
	}

	int _maOptionsBox(const wchar* title, const wchar* destructiveText, const wchar* cancelText, int bufPointer, int bufSize,
						JNIEnv* jNIEnv, jobject jThis)
	{
		Base::gSyscall->VM_Yield();

		jstring jstrTitle = jNIEnv->NewString((jchar*)title, wideCharStringLength(title));
		jstring jstrText = jNIEnv->NewString((jchar*)destructiveText, wideCharStringLength(destructiveText));
		jstring jstrCancelText = jNIEnv->NewString((jchar*)cancelText, wideCharStringLength(cancelText));

		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(cls, "maOptionsBox", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;II)I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID, jstrTitle, jstrText, jstrCancelText, bufPointer, bufSize);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrTitle);
		jNIEnv->DeleteLocalRef(jstrText);
		jNIEnv->DeleteLocalRef(jstrCancelText);

		return ret;
	}

	/**
	 * Add a notification item.
	 *
	 * Note that there can only be one notification of type
	 * NOTIFICATION_TYPE_APPLICATION_LAUNCHER. Additional notification
	 * types may be added in the future. This syscall is available
	 * on Android only.
	 *
	 * @param type The \link #NOTIFICATION_TYPE_APPLICATION_LAUNCHER
	 * \endlink constant.
	 * @param id The id of the notification. The id must be unique within
	 * the application.
	 * @param title Title of the notification.
	 * @param text String to be displayed as part of the notification.
	 * @return \< 0 on error or if the syscall is not available on the
	 * current platform.
	 */
	int _maNotificationAdd(
		int type,
		int id,
		const char* title,
		const char* text,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jstring jstrTitle = jNIEnv->NewStringUTF(title);
		jstring jstrText = jNIEnv->NewStringUTF(text);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maNotificationAdd",
			"(IILjava/lang/String;Ljava/lang/String;)I");
		if (methodID == 0) return 0;
		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			type,
			id,
			jstrTitle,
			jstrText);
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrTitle);
		jNIEnv->DeleteLocalRef(jstrText);

		return (int)result;
	}

	/**
	 * Remove a notification item.
	 * @param id The id of the notification.
	 * @return \< 0 on error.
	 */
	int _maNotificationRemove(int id, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maNotificationRemove",
			"(I)I");
		if (methodID == 0) return 0;
		jint result = jNIEnv->CallIntMethod(jThis, methodID, id);
		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	/**
	* Sends the application to the background, unless it's already there.
	* Generates a \link #EVENT_TYPE_FOCUS_LOST FOCUS_LOST \endlink event.
	* \note Only available on multi-tasking operating systems.
	*/
	int _maSendToBackground(JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maSendToBackground",
			"()I");
		if (methodID == 0) return 0;
		jint result = jNIEnv->CallIntMethod(jThis, methodID);
		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	/**
	 * Set the screen orientation.
	 * @param orientation One of the \link #SCREEN_ORIENTATION
	 * \endlink constants.
	 * @return \< 0 on error.
	 */
	int _maScreenSetOrientation(int orientation, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maScreenSetOrientation",
			"(I)I");
		if (methodID == 0) return 0;
		jint result = jNIEnv->CallIntMethod(jThis, methodID, orientation);
		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	/**
	 * Enable/disable fullscreen mode.
	 * @param fullscreen 1 for fullscreen on, 0 for fullscreen off.
	 * @return \< 0 on error.
	 */
	int _maScreenSetFullscreen(int fullscreen, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maScreenSetFullscreen",
			"(I)I");
		if (methodID == 0) return 0;
		jint result = jNIEnv->CallIntMethod(jThis, methodID, fullscreen);
		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	/**
	* Turn on/off sending of HomeScreen events. Off by default.
	* @param eventsOn 1 = events on, 0 = events off
	* @return \< 0 on error.
	*/
	int _maHomeScreenEventsOnOff(int eventsOn, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maHomeScreenEventsOnOff",
			"(I)I");
		if (methodID == 0) return 0;
		jint result = jNIEnv->CallIntMethod(jThis, methodID, eventsOn);
		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	/**
	* Add shortcut icon to the device home screen.
	* Available on Android only.
	* @param name The name (label) of the shortcut to add.
	* @return \< 0 on error.
	*/
	int _maHomeScreenShortcutAdd(const char* name, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrName = jNIEnv->NewStringUTF(name);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maHomeScreenShortcutAdd",
			"(Ljava/lang/String;)I");
		if (methodID == 0) return 0;
		jint result = jNIEnv->CallIntMethod(jThis, methodID, jstrName);
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrName);

		return (int)result;
	}

	/**
	* Remove shortcut icon from the device home screen.
	* Available on Android only.
	* @param name The name (label) of the shortcut to remove.
	* @return \< 0 on error.
	*/
	int _maHomeScreenShortcutRemove(const char* name, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrName = jNIEnv->NewStringUTF(name);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maHomeScreenShortcutRemove",
			"(Ljava/lang/String;)I");
		if (methodID == 0) return 0;
		jint result = jNIEnv->CallIntMethod(jThis, methodID, jstrName);
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrName);

		return (int)result;
	}

	/**
	* Turn on/off sending of screen on/off events. Off by default.
	* @param eventsOn 1 = events on, 0 = events off
	* @return \< 0 on error.
	*/
	int _maScreenStateEventsOnOff(int eventsOn, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maScreenStateEventsOnOff",
			"(I)I");
		if (methodID == 0) return 0;
		jint result = jNIEnv->CallIntMethod(jThis, methodID, eventsOn);
		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maWakeLock(int flag, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maWakeLock",
			"(I)I");
		if (methodID == 0) return -2;
		jint result = jNIEnv->CallIntMethod(jThis, methodID, flag);
		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maWidgetCreate(const char *widgetType, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jstring jstrWidgetType = jNIEnv->NewStringUTF(widgetType);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maWidgetCreate", "(Ljava/lang/String;)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the Java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, jstrWidgetType);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrWidgetType);

		return result;
	}

	int _maWidgetDestroy(int widget, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maWidgetDestroy", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, widget);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maWidgetAddChild(int parent, int child, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maWidgetAddChild", "(II)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, parent, child);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maWidgetInsertChild(int parent, int child, int index, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maWidgetInsertChild", "(III)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, parent, child, index);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}


	int _maWidgetRemoveChild(int child, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maWidgetRemoveChild", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, child);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}
	int _maWidgetModalDialogShow(int dialog, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maWidgetModalDialogShow", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, dialog);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maWidgetModalDialogHide(int dialog, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maWidgetModalDialogHide", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, dialog);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maWidgetSetProperty(int widget, const char *property, const char* value, JNIEnv* jNIEnv, jobject jThis)
	{
		// Convert to Java parameters
		jstring jstrProperty = jNIEnv->NewStringUTF(property);
		jstring jstrValue = jNIEnv->NewStringUTF(value);

		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maWidgetSetProperty", "(ILjava/lang/String;Ljava/lang/String;)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, widget, jstrProperty, jstrValue);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrValue);
		jNIEnv->DeleteLocalRef(jstrProperty);

		return result;
	}

	int _maWidgetGetProperty(int memStart, int widget, const char *property, int memBuffer, int bufferSize, JNIEnv* jNIEnv, jobject jThis)
	{
		// Convert to Java parameters
		jstring jstrProperty = jNIEnv->NewStringUTF(property);

		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maWidgetGetProperty", "(ILjava/lang/String;II)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, widget, jstrProperty, memBuffer - memStart, bufferSize);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrProperty);

		return result;
	}

	int _maWidgetScreenAddOptionsMenuItem(int widget, const char* title, int iconHandle, int iconPredefined, JNIEnv* jNIEnv, jobject jThis)
	{
		// Convert to Java parameters
		jstring jstrTitle = jNIEnv->NewStringUTF(title);

		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maWidgetScreenAddOptionsMenuItem", "(ILjava/lang/String;II)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, widget, jstrTitle, iconHandle, iconPredefined);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrTitle);

		return result;
	}

	int _maWidgetScreenShow(int screenWidget, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maWidgetScreenShow", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, screenWidget);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maWidgetStackScreenPush(int stackScreenWidget, int newScreen, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maWidgetStackScreenPush", "(II)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, stackScreenWidget, newScreen);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maWidgetStackScreenPop(int stackScreenWidget, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maWidgetStackScreenPop", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, stackScreenWidget);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maOpenGLInitFullscreen(int glApi, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maOpenGLInitFullscreen",
												 "(I)I");
		if (methodID == 0)
			return 0;
		jint result = jNIEnv->CallIntMethod(jThis, methodID, glApi);
		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maOpenGLCloseFullscreen(JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID =
			jNIEnv->GetMethodID(cls, "maOpenGLCloseFullscreen", "()I");
		if (methodID == 0) return 0;
		int ret = jNIEnv->CallIntMethod(jThis, methodID);
		jNIEnv->DeleteLocalRef(cls);

		return ret;
	}

	int _maOpenGLTexImage2D(MAHandle image, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "loadGlTexture",
												 "(I)I");
		if (methodID == 0)
			return 0;
		jint result = jNIEnv->CallIntMethod(jThis, methodID, image);
		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maOpenGLTexSubImage2D(MAHandle image, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "loadGlSubTexture",
												 "(I)I");
		if (methodID == 0)
			return 0;
		jint result = jNIEnv->CallIntMethod(jThis, methodID, image);
		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}




	MAHandle _maFileOpen(
		const char* path,
		int mode,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jstring jstrPath = jNIEnv->NewStringUTF(path);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileOpen",
												 "(Ljava/lang/String;I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, jstrPath, mode);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrPath);

		return (int)result;
	}

	int _maFileExists(MAHandle file, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileExists",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, file);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileClose(MAHandle file, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileClose",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, file);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileCreate(MAHandle file, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileCreate",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, file);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileDelete(MAHandle file, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileDelete",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, file);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileSize(MAHandle file, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileSize",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, file);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileAvailableSpace(MAHandle file, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileAvailableSpace",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, file);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileTotalSpace(MAHandle file, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileTotalSpace",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, file);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileDate(MAHandle file, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileDate",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, file);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileRename(
		MAHandle file,
		const char* newName,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jstring jstrNewName = jNIEnv->NewStringUTF(newName);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maFileRename",
			"(ILjava/lang/String;)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID, file, jstrNewName);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrNewName);

		return (int)result;
	}

	int _maFileTruncate(
		MAHandle file,
		int offset,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileTruncate",
												 "(II)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, file, offset);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileWrite(
		MAHandle file,
		int src,
		int len,
		int memStart,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedSrc = src - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileWrite",
												 "(III)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, file,
											fixedSrc, len);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileWriteFromData(
		MAHandle file,
		MAHandle data,
		int offset,
		int len,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileWriteFromData",
												 "(IIII)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, file,
											data, offset, len);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileRead(
		MAHandle file,
		int dst,
		int len,
		int memStart,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedDst = dst - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileRead",
												 "(III)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, file, fixedDst, len);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileReadToData(
		MAHandle file,
		MAHandle data,
		int offset,
		int len,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileReadToData",
												 "(IIII)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, file,
											data, offset, len);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileTell(MAHandle file, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileTell",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, file);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileSeek(
		MAHandle file,
		int offset,
		int whence,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileSeek",
												 "(III)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID,
											file, offset, whence);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	MAHandle _maFileListStart(
		const char* path,
		const char* filter,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jstring jstrPath = jNIEnv->NewStringUTF(path);
		jstring jstrFilter = jNIEnv->NewStringUTF(filter);

		jmethodID methodID = jNIEnv->GetMethodID(
								cls,
								"maFileListStart",
								"(Ljava/lang/String;Ljava/lang/String;)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID,
											jstrPath, jstrFilter);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrPath);
		jNIEnv->DeleteLocalRef(jstrFilter);

		return (int)result;
	}

	int _maFileListNext(
		MAHandle list,
		int nameBuf,
		int bufSize,
		int memStart,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedNameBuf = nameBuf - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileListNext",
												 "(III)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID,
											list, nameBuf, bufSize);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maFileListClose(MAHandle list, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFileListClose",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, list);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maCameraStart(JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCameraStart", "()I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the Java method
		int result = jNIEnv->CallIntMethod(jThis, methodID);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maCameraStop(JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCameraStop", "()I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the Java method
		int result = jNIEnv->CallIntMethod(jThis, methodID);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maCameraSetPreview(MAHandle widgetHandle, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCameraSetPreview", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the Java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, widgetHandle);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maCameraSelect(MAHandle cameraHandle, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCameraSelect", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the Java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, cameraHandle);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maCameraNumber(JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCameraNumber", "()I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the Java method
		int result = jNIEnv->CallIntMethod(jThis, methodID);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}


	int _maCameraSnapshot(int formatIndex, MAHandle placeHolder, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCameraSnapshot", "(II)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the Java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, formatIndex, placeHolder);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maCameraRecord(int flag, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCameraRecord", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the Java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, flag);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}


	int _maCameraFormatNumber(JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCameraFormatNumber", "()I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the Java method
		int result = jNIEnv->CallIntMethod(jThis, methodID);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maCameraFormat(int index, int sizeInfoBuffer, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCameraFormat", "(II)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the Java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, index, sizeInfoBuffer);

		// Delete allocated memory

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maCameraSetProperty(const char *property, const char* value, JNIEnv* jNIEnv, jobject jThis)
	{
		// Convert to Java parameters
		jstring jstrProperty = jNIEnv->NewStringUTF(property);
		jstring jstrValue = jNIEnv->NewStringUTF(value);

		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCameraSetProperty", "(Ljava/lang/String;Ljava/lang/String;)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, jstrProperty, jstrValue);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrValue);
		jNIEnv->DeleteLocalRef(jstrProperty);

		return result;
	}

	int _maCameraGetProperty(int memStart, const char *property, int memBuffer, int bufferSize, JNIEnv* jNIEnv, jobject jThis)
	{
		// Convert to Java parameters
		jstring jstrProperty = jNIEnv->NewStringUTF(property);

		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCameraGetProperty", "(Ljava/lang/String;II)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, jstrProperty, memBuffer - memStart, bufferSize);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrProperty);

		return result;
	}

	//////////////
	//  SENSOR  //
	//////////////
	int _maSensorStart(int sensor, int interval,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(cls, "maSensorStart", "(II)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			sensor, interval);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maSensorStop(int sensor,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(cls, "maSensorStop", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			sensor);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	//////////////////////
	//		PIM			//
	//////////////////////
	MAHandle _maPimListOpen(int listType,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimListOpen", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			listType);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	MAHandle _maPimListNext(MAHandle list,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimListNext", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			list);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	/**
	* Returns a handle to one of the default fonts of the device, in the style and size you specify.
	* \param 'type' The type of the font, can be FONT_TYPE_[SANS_SERIF,SERIF,MONOSPACE].
	* \param 'style' The style of the font, can be FONT_STYLE_[NORMAL,BOLD,ITALIC].
	* \param 'size' The size of the font.
	* \return The handle to the font, RES_FONT_NO_TYPE_STYLE_COMBINATION, or RES_FONT_INVALID_SIZE.
	*/
	MAHandle _maFontLoadDefault(int type, int style, int size, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFontLoadDefault",
												 "(III)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, type,style,size);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	/**
	* Sets the font to be used with maDrawText and maDrawTextW, and returns the handle
	* to the previous font
	* \param 'font' an MAHandle for a font object
	* \return The handle to the previous font, or RES_FONT_INVALID_HANDLE
	*/
	MAHandle _maFontSetCurrent(MAHandle font, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFontSetCurrent",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, font);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;

	}

	/**
	* Returns the number of fonts that are available in the system
	* \return Number of fonts
	* \see maFontGetName
	*/
	int _maFontGetCount(JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFontGetCount",
												 "()I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	/**
	* Copies the font postscript name of the given index to the buffer.
	* You must have first called maFontGetCount() at least once before calling this function.
	* \param 'index' A 0-based index to the font
	* \param 'buffer' An empty char buffer that will receive the font name
	* \param 'bufferLen' The size of the buffer
	* \return The number of bytes copied (with terminating NULL) or RES_FONT_INDEX_OUT_OF_BOUNDS
	* or RES_FONT_INSUFFICIENT_BUFFER or RES_FONT_LIST_NOT_INITIALIZED
	* \see maFontGetCount, maFontLoadWithName
	*/
	int _maFontGetName(int memStart, int index, int memBuffer, int bufferSize, JNIEnv* jNIEnv, jobject jThis)
	{
		// Get the Java method
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maFontGetName", "(III)I");
		if (methodID == 0)
		{
			return 0;
		}

		// Call the java method
		int result = jNIEnv->CallIntMethod(jThis, methodID, index, memBuffer - memStart, bufferSize);

		// Delete allocated memory
		jNIEnv->DeleteLocalRef(cls);

		return result;

	}

	/**
	* Returns a handle to a font with the specific postscript name and size
	* \param 'postScriptName' The postscript name of the font
	* \param 'size' The size of the font
	* \return Handle to the font, RES_FONT_NAME_NONEXISTENT, or RES_FONT_INVALID_SIZE.
	*/
	MAHandle _maFontLoadWithName(
			const char* postScriptName,
			int size,
			JNIEnv* jNIEnv,
			jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jstring jstrBuf = jNIEnv->NewStringUTF(postScriptName);
		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFontLoadWithName",
												 "(Ljava/lang/String;I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, jstrBuf, size);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrBuf);
		return (int)result;
	}

	/**
	* Deletes a loaded font
	* \param 'font' A font handle
	* \return RES_FONT_OK or RES_FONT_INVALID_HANDLE
	*/
	int _maFontDelete(MAHandle font, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maFontDelete",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, font);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maPimListClose(MAHandle list,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimListClose", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			list);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maPimItemCount(MAHandle item,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimItemCount", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			item);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maPimItemGetField(MAHandle item, int n,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimItemGetField", "(II)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			item, n);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maPimItemFieldCount(MAHandle item, int field,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimItemFieldCount", "(II)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			item, field);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maPimItemGetAttributes(MAHandle item, int field, int index,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimItemGetAttributes", "(III)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			item, field, index);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maPimItemSetLabel(int item, int field, int bufPointer, int bufSize, int index,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimItemSetLabel", "(IIIII)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			item, field, bufPointer, bufSize, index);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maPimItemGetLabel(int item, int field, int bufPointer, int bufSize, int index,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimItemGetLabel", "(IIIII)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			item, field, bufPointer, bufSize, index);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maPimFieldType(MAHandle list, int field,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimFieldType", "(II)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			list, field);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maPimItemGetValue(int item, int field, int bufPointer, int bufSize, int index,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimItemGetValue", "(IIIII)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			item, field, bufPointer, bufSize, index);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maPimItemSetValue(int item, int field, int bufPointer, int bufSize, int index, int attributes,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimItemSetValue", "(IIIIII)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			item, field, bufPointer, bufSize, index, attributes);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maPimItemAddValue(int item, int field, int bufPointer, int bufSize, int attributes,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimItemAddValue", "(IIIII)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			item, field, bufPointer, bufSize, attributes);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maPimItemRemoveValue(MAHandle item, int field, int index,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimItemRemoveValue", "(III)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			item, field, index);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maPimItemClose(MAHandle item,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimItemClose", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			item);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	MAHandle _maPimItemCreate(MAHandle list,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimItemCreate", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			list);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maPimItemRemove(MAHandle list, MAHandle item,
		JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maPimItemRemove", "(II)I");
		if (methodID == 0)
		{
			return 0;
		}

		jint result = jNIEnv->CallIntMethod(
			jThis, methodID,
			list, item);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}


	int _maNFCStart(JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCStart",
												 "()I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCStop(JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCStop",
												 "()V");
		if (methodID == 0)
			return 0;

		jNIEnv->CallVoidMethod(jThis, methodID);

		jNIEnv->DeleteLocalRef(cls);

		return 1;
	}

	MAHandle _maNFCReadTag(MAHandle nfcContext, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCReadTag",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, nfcContext);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCDestroyTag(MAHandle tagHandle, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCDestroyTag",
												 "(I)V");
		if (methodID == 0)
			return 0;

		jNIEnv->CallVoidMethod(jThis, methodID, tagHandle);

		jNIEnv->DeleteLocalRef(cls);

		return 1;
	}

	int _maNFCConnectTag(MAHandle tagHandle, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCConnectTag",
												 "(I)V");
		if (methodID == 0)
			return 0;

		jNIEnv->CallVoidMethod(jThis, methodID, tagHandle);

		jNIEnv->DeleteLocalRef(cls);

		return 1;
	};

	int _maNFCCloseTag(MAHandle tagHandle, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCCloseTag",
												 "(I)V");
		if (methodID == 0)
			return 0;

		jNIEnv->CallVoidMethod(jThis, methodID, tagHandle);

		jNIEnv->DeleteLocalRef(cls);

		return 1;
	};

	MAHandle _maNFCBatchStart(MAHandle tagHandle, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCBatchStart",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, tagHandle);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCBatchCommit(MAHandle tagHandle, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCBatchCommit",
												 "(I)V");
		if (methodID == 0)
			return 0;

		jNIEnv->CallVoidMethod(jThis, methodID, tagHandle);

		jNIEnv->DeleteLocalRef(cls);

		return 1;
	}

	int _maNFCBatchRollback(MAHandle tagHandle, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCBatchRollback",
												 "(I)V");
		if (methodID == 0)
			return 0;

		jNIEnv->CallVoidMethod(jThis, methodID, tagHandle);

		jNIEnv->DeleteLocalRef(cls);

		return 1;
	}

	int _maNFCTransceive(MAHandle tagHandle, int src, int srcLen, int dst, int dstLen, int memStart, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedSrc = src - memStart;
		int fixedDst = dst - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCTransceive",
												 "(IIIIII)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, tagHandle, fixedSrc, srcLen, fixedDst, dstLen, dst);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCGetSize(MAHandle tagHandle, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCGetSize",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, tagHandle);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	MAHandle _maNFCGetNDEFMessage(MAHandle tagHandle, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCGetNDEFMessage",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, tagHandle);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCReadNDEFMessage(MAHandle tagHandle, JNIEnv* jNIEnv, jobject jThis)  {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCReadNDEFMessage",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, tagHandle);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCWriteNDEFMessage(MAHandle tagHandle, MAHandle ndefMessage, JNIEnv* jNIEnv, jobject jThis)  {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCWriteNDEFMessage",
												 "(II)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, tagHandle, ndefMessage);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	MAHandle _maNFCCreateNDEFMessage(int recordCount, JNIEnv* jNIEnv, jobject jThis)  {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCCreateNDEFMessage",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, recordCount);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	MAHandle _maNFCGetNDEFRecord(MAHandle ndefHandle, int ix, JNIEnv* jNIEnv, jobject jThis)  {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCGetNDEFRecord",
												 "(II)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, ndefHandle, ix);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCGetNDEFRecordCount(MAHandle ndefHandle, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCGetNDEFRecordCount",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, ndefHandle);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCGetNDEFId(MAHandle ndefRecordHandle, int dst, int len, int memStart, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedDst = dst == NULL ? 0 : dst - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCGetNDEFId",
												 "(III)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, ndefRecordHandle, fixedDst, len);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCGetNDEFPayload(MAHandle ndefRecordHandle, int dst, int len, int memStart, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedDst = dst == NULL ? 0 : dst - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCGetNDEFPayload",
												 "(III)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, ndefRecordHandle, fixedDst, len);

		return result;
	}

	int _maNFCGetNDEFTnf(MAHandle ndefRecordHandle, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCGetNDEFTnf",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, ndefRecordHandle);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCGetNDEFType(MAHandle ndefRecordHandle, int dst, int len, int memStart, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedDst = dst == NULL ? 0 : dst - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCGetNDEFType",
												 "(III)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, ndefRecordHandle, fixedDst, len);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCSetNDEFId(MAHandle ndefRecordHandle, int src, int len, int memStart, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedSrc = src - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCSetNDEFId",
												 "(III)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, ndefRecordHandle, fixedSrc, len);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCSetNDEFPayload(MAHandle ndefRecordHandle, int src, int len, int memStart, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedSrc = src - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCSetNDEFPayload",
												 "(III)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, ndefRecordHandle, fixedSrc, len);

		return result;
	}

	int _maNFCSetNDEFTnf(MAHandle ndefRecordHandle, int tnf, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCSetNDEFTnf",
												 "(II)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, ndefRecordHandle, tnf);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCSetNDEFType(MAHandle ndefRecordHandle, int src, int len, int memStart, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedSrc = src - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCSetNDEFType",
												 "(III)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, ndefRecordHandle, fixedSrc, len);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCIsType(MAHandle tagHandle, int tagType, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCIsType",
												 "(II)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, tagHandle, tagType);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCGetTypedTag(MAHandle tagHandle, int tagType, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCGetTypedTag",
												 "(II)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, tagHandle, tagType);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCAuthenticateMifareSector(MAHandle mfcTag, int keyType, int sectorIndex, int keyAddr, int keyLen, int memStart, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedKeyAddr = keyAddr - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCAuthenticateMifareSector",
												 "(IIIII)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, mfcTag, keyType, sectorIndex, fixedKeyAddr, keyLen);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCGetMifareSectorCount(MAHandle mfcTag, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCGetMifareSectorCount",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, mfcTag);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCGetMifareBlockCountInSector(MAHandle mfcTag, int sectorIndex, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCGetMifareBlockCountInSector",
												 "(II)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, mfcTag, sectorIndex);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCMifareSectorToBlock(MAHandle mfcTag, int sectorIndex, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCMifareSectorToBlock",
												 "(II)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, mfcTag, sectorIndex);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCReadMifareBlocks(MAHandle mfcTag, int firstBlock, int dst, int len, int memStart, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedDst = dst - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCReadMifareBlocks",
												 "(IIII)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, mfcTag, firstBlock, fixedDst, len);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCReadMifarePages(MAHandle mfcTag, int firstPage, int dst, int len, int memStart, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedDst = dst - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCReadMifarePages",
												 "(IIII)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, mfcTag, firstPage, fixedDst, len);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCWriteMifareBlocks(MAHandle mfcTag, int firstBlock, int src, int len, int memStart, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedSrc = src - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCWriteMifareBlocks",
												 "(IIII)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, mfcTag, firstBlock, fixedSrc, len);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCWriteMifarePages(MAHandle mfcTag, int firstPage, int src, int len, int memStart, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		int fixedSrc = src - memStart;

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCWriteMifarePages",
												 "(IIII)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, mfcTag, firstPage, fixedSrc, len);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCSetReadOnly(MAHandle tag, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCSetReadOnly",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, tag);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maNFCIsReadOnly(MAHandle tag, JNIEnv* jNIEnv, jobject jThis) {
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
												 cls,
												 "maNFCIsReadOnly",
												 "(I)I");
		if (methodID == 0)
			return 0;

		jint result = jNIEnv->CallIntMethod(jThis, methodID, tag);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maAdsBannerCreate(int bannerSize, const char* publisherID, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrPublisher = jNIEnv->NewStringUTF(publisherID);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maAdsBannerCreate", "(ILjava/lang/String;)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, bannerSize, jstrPublisher);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrPublisher);

		return result;
	}

	int _maAdsAddBannerToLayout(MAHandle bannerHandle, MAHandle layoutHandle, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maAdsAddBannerToLayout", "(II)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, bannerHandle, layoutHandle);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maAdsRemoveBannerFromLayout(MAHandle bannerHandle, MAHandle layoutHandle, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maAdsRemoveBannerFromLayout", "(II)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, bannerHandle, layoutHandle);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maAdsBannerDestroy(MAHandle bannerHandle, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maAdsBannerDestroy", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, bannerHandle);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maAdsBannerSetProperty(MAHandle bannerHandle, const char* property, const char* value, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrProp = jNIEnv->NewStringUTF(property);
		jstring jstrValue = jNIEnv->NewStringUTF(value);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maAdsBannerSetProperty", "(ILjava/lang/String;Ljava/lang/String;)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, bannerHandle, jstrProp, jstrValue);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrValue);
		jNIEnv->DeleteLocalRef(jstrProp);

		return result;
	}

	int _maAdsBannerGetProperty(int memStart, MAHandle bannerHandle, const char* property, int memBuffer, int bufSize, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrProp = jNIEnv->NewStringUTF(property);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maAdsBannerGetProperty", "(ILjava/lang/String;II)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, bannerHandle, jstrProp, memBuffer - memStart, bufSize);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrProp);

		return result;
	}

	int _maNotificationLocalCreate(JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maNotificationLocalCreate", "()I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maNotificationLocalDestroy(MAHandle notificationHandle, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maNotificationLocalDestroy", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, notificationHandle);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maNotificationLocalSetProperty(MAHandle notificationHandle, const char* property, const char* value, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrProp = jNIEnv->NewStringUTF(property);
		jstring jstrValue = jNIEnv->NewStringUTF(value);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maNotificationLocalSetProperty", "(ILjava/lang/String;Ljava/lang/String;)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, notificationHandle, jstrProp, jstrValue);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrValue);
		jNIEnv->DeleteLocalRef(jstrProp);

		return result;
	}

	int _maNotificationLocalGetProperty(int memStart, MAHandle notificationHandle, const char* property, int memBuffer, int bufSize, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrProp = jNIEnv->NewStringUTF(property);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maNotificationLocalGetProperty", "(ILjava/lang/String;II)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, notificationHandle, jstrProp, memBuffer - memStart, bufSize);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrProp);

		return result;
	}

	int _maNotificationLocalSchedule(MAHandle notificationHandle, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maNotificationLocalSchedule", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, notificationHandle);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maNotificationLocalUnschedule(MAHandle notificationHandle, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maNotificationLocalUnschedule", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, notificationHandle);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maNotificationPushRegister(MAHandle pushNotificationTypes, const char* accountID, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jstring jstr = jNIEnv->NewStringUTF(accountID);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maNotificationPushRegister", "(ILjava/lang/String;)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, pushNotificationTypes, jstr);

		jNIEnv->DeleteLocalRef(jstr);
		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maNotificationPushGetRegistration(int memStart, int messagePointer, int messageSize, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maNotificationPushGetRegistration", "(II)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, messagePointer - memStart, messageSize);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maNotificationPushUnregister(JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maNotificationPushUnregister", "()I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maNotificationPushGetData(MAHandle pushNotificationHandle, int memStart, int bufferPointer, int messageSize, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		//int fixedDst = destination == NULL ? 0 : destination - memStart;
		//int dest = (int)messagePointer - (int)gCore->mem_ds;

		jmethodID methodID = jNIEnv->GetMethodID(cls, "maNotificationPushGetData", "(III)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, pushNotificationHandle, bufferPointer - memStart, messageSize);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maNotificationPushDestroy(MAHandle pushNotificationHandle, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maNotificationPushDestroy", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, pushNotificationHandle);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maNotificationPushSetTickerText(const char* text, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrText = jNIEnv->NewStringUTF(text);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maNotificationPushSetTickerText", "(Ljava/lang/String;)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, jstrText);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrText);

		return result;
	}

	int _maNotificationPushSetMessageTitle(const char* text, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrText = jNIEnv->NewStringUTF(text);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maNotificationPushSetMessageTitle", "(Ljava/lang/String;)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, jstrText);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrText);

		return result;
	}

	int _maNotificationPushSetDisplayFlag(int flag, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maNotificationPushSetDisplayFlag", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, flag);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maCaptureSetProperty(const char* property, const char* value, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrProp = jNIEnv->NewStringUTF(property);
		jstring jstrValue = jNIEnv->NewStringUTF(value);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCaptureSetProperty", "(Ljava/lang/String;Ljava/lang/String;)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, jstrProp, jstrValue);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrValue);
		jNIEnv->DeleteLocalRef(jstrProp);

		return result;
	}

	int _maCaptureGetProperty(int memStart, const char* property, int memBuffer, int bufSize, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrProp = jNIEnv->NewStringUTF(property);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCaptureGetProperty", "(Ljava/lang/String;II)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, jstrProp, memBuffer - memStart, bufSize);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrProp);

		return result;
	}

	int _maCaptureAction(int action, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCaptureAction", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, action);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maCaptureWriteImage(MAHandle handle, const char* fullPathBuffer, int fullPathBufSize, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrValue = jNIEnv->NewStringUTF(fullPathBuffer);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCaptureWriteImage", "(ILjava/lang/String;I)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, handle, jstrValue, fullPathBufSize);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrValue);

		return result;
	}

	int _maCaptureGetImagePath(int memStart, MAHandle handle, int buffer, int bufferSize, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCaptureGetImagePath", "(III)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, handle, buffer - memStart, bufferSize);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maCaptureGetVideoPath(int memStart, MAHandle handle, int buffer, int bufferSize, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCaptureGetVideoPath", "(III)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, handle, buffer - memStart, bufferSize);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maCaptureDestroyData(MAHandle handle, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maCaptureDestroyData", "(I)I");
		if (methodID == 0)
		{
			return 0;
		}

		int result = jNIEnv->CallIntMethod(jThis, methodID, handle);

		jNIEnv->DeleteLocalRef(cls);

		return result;
	}

	int _maSyscallPanicsEnable(JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maSyscallPanicsEnable", "()I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID);
		jNIEnv->DeleteLocalRef(cls);

		return (int)ret;
	}

	int _maSyscallPanicsDisable(JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maSyscallPanicsEnable", "()I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID);
		jNIEnv->DeleteLocalRef(cls);

		return (int)ret;
	}

	int _maGetCellInfo(MAHandle mem, int memStart, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(cls, "maGetCellInfo", "(I)I");
		if (methodID == 0) return 0;
		jint ret = jNIEnv->CallIntMethod(jThis, methodID, (mem-memStart));
		jNIEnv->DeleteLocalRef(cls);
		return (int)ret;
	}


	// ********** Database API **********

	/**
	 * Open a database, the database is created if it does not exist.
	 * @param path Full path to database file name.
	 * @return Handle to the database >0 on success, #MA_DB_ERROR on error.
	 */
	int _maDBOpen(const char* path, JNIEnv* jNIEnv, jobject jThis)
	{
		jstring jstrPath = jNIEnv->NewStringUTF(path);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maDBOpen",
			"(Ljava/lang/String;)I");
		if (methodID == 0)
		{
			// Method not found.
			return -1;
		}
		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			jstrPath);
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrPath);
		return (int)result;
	}

	/**
	 * Close a database.
	 * @param databaseHandle Handle to the database.
	 * @return #MA_DB_OK on success, #MA_DB_ERROR on error.
	 */
	int _maDBClose(MAHandle databaseHandle, JNIEnv* jNIEnv, jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maDBClose",
			"(I)I");
		if (methodID == 0)
		{
			// Method not found.
			return -1;
		}
		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			databaseHandle);
		jNIEnv->DeleteLocalRef(cls);
		return (int)result;
	}

	/**
	 * Executes an SQL statement. If the statement returns a
	 * query result, a cursor handle is returned.
	 * @param databaseHandle Handle to the database.
	 * @param sql The SQL statement.
	 * @return #MA_DB_ERROR on error, #MA_DB_OK on success,
	 * > 0 if there is a cursor to a query result, in this
	 * case the return value is the cursor handle.
	 */
	int _maDBExecSQL(
		MAHandle databaseHandle,
		const char* sql,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jstring jstrSql = jNIEnv->NewStringUTF(sql);
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maDBExecSQL",
			"(ILjava/lang/String;)I");
		if (methodID == 0)
		{
			// Method not found.
			return -1;
		}
		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			databaseHandle,
			jstrSql);
		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrSql);
		return (int)result;
	}

	/**
	 * Destroys a cursor. You must call this function
	 * when you are done with the cursor to release
	 * its resources.
	 * @param cursorHandle Handle to the cursor.
	 * @return #MA_DB_OK on success, #MA_DB_ERROR on error.
	 */
	int _maDBCursorDestroy(
		MAHandle cursorHandle,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maDBCursorDestroy",
			"(I)I");
		if (methodID == 0)
		{
			// Method not found.
			return -1;
		}
		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			cursorHandle);
		jNIEnv->DeleteLocalRef(cls);
		return (int)result;
	}

	/**
	 * Move the cursor to the next row in the result set.
	 * Note that you must call this function before retrieving
	 * column data. The initial position of the cursor is
	 * before the first row in the result set. If the result
	 * set is empty, this function will return a value != MA_DB_OK.
	 * @param cursorHandle Handle to the cursor.
	 * @return #MA_DB_OK if successfully moved to next row,
	 * #MA_DB_NO_ROW if there are no more rows in the result set,
	 * #MA_DB_ERROR on error.
	 */
	int _maDBCursorNext(
		MAHandle cursorHandle,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maDBCursorNext",
			"(I)I");
		if (methodID == 0)
		{
			// Method not found.
			return -1;
		}
		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			cursorHandle);
		jNIEnv->DeleteLocalRef(cls);
		return (int)result;
	}

	/**
	 * Get the column value at the current row pointed to
	 * by the cursor as a data object. Use this function for
	 * blob data or text data.
	 * @param cursorHandle Handle to the cursor.
	 * @param columnIndex Index of the column to retrieve value from.
	 * First column has index zero.
	 * @param placeholder Handle created with maCreatePlaceholder.
	 * A data object will be created with the column data, and the handle
	 * will refer to that data.
	 * @return #MA_DB_OK on success, #MA_DB_ERROR on error.
	 */
	int _maDBCursorGetColumnData(
		MAHandle cursorHandle,
		int columnIndex,
		MAHandle placeholder,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maDBCursorGetColumnData",
			"(III)I");
		if (methodID == 0)
		{
			// Method not found.
			return -1;
		}
		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			cursorHandle,
			columnIndex,
			placeholder);
		jNIEnv->DeleteLocalRef(cls);
		return (int)result;
	}

	/**
	 * Get the column value at the current row pointed to
	 * by the cursor as a text data buffer. Use this function for
	 * text data.
	 * @param cursorHandle Handle to the cursor.
	 * @param columnIndex Index of the column to retrieve value from.
	 * First column has index zero.
	 * @param bufferAddress Address to buffer to receive the data.
	 * The result is NOT zero terminated.
	 * @param bufferSize Max size of the buffer.
	 * @return The actual length of the data, if the actual length
	 * returned is > bufferSize, data was not copied (buffer too small),
	 * returns #MA_DB_ERROR on other errors.
	 */
	int _maDBCursorGetColumnText(
		MAHandle cursorHandle,
		int columnIndex,
		int bufferAddress,
		int bufferSize,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maDBCursorGetColumnText",
			"(IIII)I");
		if (methodID == 0)
		{
			// Method not found.
			return -1;
		}
		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			cursorHandle,
			columnIndex,
			bufferAddress,
			bufferSize);
		jNIEnv->DeleteLocalRef(cls);
		return (int)result;
	}

	/**
	 * Get the column value at the current row pointed to
	 * by the cursor as an int value.
	 * @param cursorHandle Handle to the cursor.
	 * @param columnIndex Index of the column to retrieve value from.
	 * First column has index zero.
	 * @param intValueAddress Address to int to receive the value.
	 * @return #MA_DB_OK on success, #MA_DB_ERROR on error.
	 */
	int _maDBCursorGetColumnInt(
		MAHandle cursorHandle,
		int columnIndex,
		int intValueAddress,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maDBCursorGetColumnInt",
			"(III)I");
		if (methodID == 0)
		{
			// Method not found.
			return -1;
		}
		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			cursorHandle,
			columnIndex,
			intValueAddress);
		jNIEnv->DeleteLocalRef(cls);
		return (int)result;
	}

	/**
	 * Get the column value at the current row pointed to
	 * by the cursor as a double value.
	 * @param cursorHandle Handle to the cursor.
	 * @param columnIndex Index of the column to retrieve value from.
	 * First column has index zero.
	 * @param doubleValueAddress Address to double to receive the value.
	 * @return #MA_DB_OK on success, #MA_DB_ERROR on error.
	 */
	int _maDBCursorGetColumnDouble(
		MAHandle cursorHandle,
		int columnIndex,
		int doubleValueAddress,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);
		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maDBCursorGetColumnDouble",
			"(III)I");
		if (methodID == 0)
		{
			// Method not found.
			return -1;
		}
		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			cursorHandle,
			columnIndex,
			doubleValueAddress);
		jNIEnv->DeleteLocalRef(cls);
		return (int)result;
	}

	// AUDIO API

	int _maAudioDataCreateFromResource(
		const char* mime,
		int data,
		int offset,
		int length,
		int flags,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jstring jstrMime = jNIEnv->NewStringUTF(mime);
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maAudioDataCreateFromResource",
			"(Ljava/lang/String;IIII)I");

		if (methodID == 0)
			return -1;

		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			jstrMime,
			data,
			offset,
			length,
			flags);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrMime);

		return (int)result;
	}

	int _maAudioDataCreateFromURL(
		const char* mime,
		const char* url,
		int flags,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jstring jstrMime = jNIEnv->NewStringUTF(mime);
		jstring jstrURL = jNIEnv->NewStringUTF(url);
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maAudioDataCreateFromURL",
			"(Ljava/lang/String;Ljava/lang/String;I)I");

		if (methodID == 0)
			return -1;

		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			jstrMime,
			jstrURL,
			flags);

		jNIEnv->DeleteLocalRef(cls);
		jNIEnv->DeleteLocalRef(jstrURL);
		jNIEnv->DeleteLocalRef(jstrMime);

		return (int)result;
	}

	int _maAudioDataDestroy(
		int audioData,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maAudioDataDestroy",
			"(I)I");

		if (methodID == 0)
			return -1;

		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			audioData);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maAudioInstanceCreate(
		int audioData,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maAudioInstanceCreate",
			"(I)I");

		if (methodID == 0)
			return -1;

		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			audioData);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maAudioInstanceDestroy(
		int audioInstance,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maAudioInstanceDestroy",
			"(I)I");

		if (methodID == 0)
			return -1;

		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			audioInstance);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maAudioGetLength(
		int audioInstance,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maAudioGetLength",
			"(I)I");

		if (methodID == 0)
			return -1;

		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			audioInstance);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maAudioSetNumberOfLoops(
		int audioInstance,
		int loops,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maAudioSetNUmberOfLoops",
			"(II)I");

		if (methodID == 0)
			return -1;

		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			audioInstance,
			loops);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maAudioPrepare(
		int audioInstance,
		int async,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maAudioPrepare",
			"(II)I");

		if (methodID == 0)
			return -1;

		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			audioInstance,
			async);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maAudioPlay(
		int audioInstance,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maAudioPlay",
			"(I)I");

		if (methodID == 0)
			return -1;

		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			audioInstance);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maAudioSetPosition(
		int audioInstance,
		int milliseconds,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maAudioSetPosition",
			"(II)I");

		if (methodID == 0)
			return -1;

		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			audioInstance,
			milliseconds);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maAudioGetPosition(
		int audioInstance,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maAudioGetPosition",
			"(I)I");

		if (methodID == 0)
			return -1;

		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			audioInstance);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maAudioSetVolume(
		int audioInstance,
		float volume,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maAudioInstanceDestroy",
			"(IF)I");

		if (methodID == 0)
			return -1;

		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			audioInstance,
			volume);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maAudioPause(
		int audioInstance,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maAudioPause",
			"(I)I");

		if (methodID == 0)
			return -1;

		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			audioInstance);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

	int _maAudioStop(
		int audioInstance,
		JNIEnv* jNIEnv,
		jobject jThis)
	{
		jclass cls = jNIEnv->GetObjectClass(jThis);

		jmethodID methodID = jNIEnv->GetMethodID(
			cls,
			"maAudioStop",
			"(I)I");

		if (methodID == 0)
			return -1;

		jint result = jNIEnv->CallIntMethod(
			jThis,
			methodID,
			audioInstance);

		jNIEnv->DeleteLocalRef(cls);

		return (int)result;
	}

}
