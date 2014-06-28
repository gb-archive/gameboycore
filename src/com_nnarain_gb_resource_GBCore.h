/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_nnarain_gb_resource_GBCore */

#ifndef _Included_com_nnarain_gb_resource_GBCore
#define _Included_com_nnarain_gb_resource_GBCore
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_nnarain_gb_resource_GBCore
 * Method:    createCore
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_nnarain_gb_resource_GBCore_createCore
  (JNIEnv *, jclass);

/*
 * Class:     com_nnarain_gb_resource_GBCore
 * Method:    loadBanks
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_nnarain_gb_resource_GBCore_loadBanks
  (JNIEnv *, jclass, jlong, jlong);

/*
 * Class:     com_nnarain_gb_resource_GBCore
 * Method:    loadROMBank0
 * Signature: (J[B)V
 */
JNIEXPORT void JNICALL Java_com_nnarain_gb_resource_GBCore_loadROMBank0
  (JNIEnv *, jclass, jlong, jbyteArray);

/*
 * Class:     com_nnarain_gb_resource_GBCore
 * Method:    swapROM
 * Signature: (J[B)V
 */
JNIEXPORT void JNICALL Java_com_nnarain_gb_resource_GBCore_swapROM
  (JNIEnv *, jclass, jlong, jbyteArray);

/*
 * Class:     com_nnarain_gb_resource_GBCore
 * Method:    step
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_nnarain_gb_resource_GBCore_step
  (JNIEnv *, jclass, jlong);

#ifdef __cplusplus
}
#endif
#endif
