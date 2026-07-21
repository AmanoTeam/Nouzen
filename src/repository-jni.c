#include <jni.h>
#include <stdlib.h>
#include "repository.h"
#include "options.h"
#include "os/cpuinfo.h"
#include "os/osdetect.h"

static jstring string_from_c(JNIEnv* env, char* str) {
	if (str == NULL) {
		return NULL;
	}
	jstring result = (*env)->NewStringUTF(env, str);
	return result;
}

// ==================== Config ====================

jint repoSetConfigDir(JNIEnv* env, jclass cls, jstring directory) {
	const char* c_dir = (*env)->GetStringUTFChars(env, directory, NULL);
	jint result = (jint) repo_set_config_dir(c_dir);
	(*env)->ReleaseStringUTFChars(env, directory, c_dir);
	return result;
}

jstring repoGetConfigDir(JNIEnv* env, jclass cls) {
	char* dir = repo_get_config_dir();
	jstring result = string_from_c(env, dir);
	free(dir);
	return result;
}

// ==================== RepoList lifecycle ====================

jlong repolistCreate(JNIEnv* env, jclass cls) {
	repolist_t* list = calloc(1, sizeof(repolist_t));
	return (jlong) list;
}

void repolistFree(JNIEnv* env, jclass cls, jlong listPtr) {
	if (listPtr == 0) return;
	repolist_t* list = (repolist_t*) listPtr;
	repolist_free(list);
	free(list);
}

jint repolistLoad(JNIEnv* env, jclass cls, jlong listPtr) {
	repolist_t* list = (repolist_t*) listPtr;
	return (jint) repolist_load(list);
}

jint repolistDestroy(JNIEnv* env, jclass cls, jlong listPtr) {
	repolist_t* list = (repolist_t*) listPtr;
	return (jint) repolist_destroy(list);
}

// ==================== RepoList queries ====================

jlong repolistGetSize(JNIEnv* env, jclass cls, jlong listPtr) {
	repolist_t* list = (repolist_t*) listPtr;
	return (jlong) list->offset;
}

jlong repolistGetRepo(JNIEnv* env, jclass cls, jlong listPtr, jlong index) {
	repolist_t* list = (repolist_t*) listPtr;
	if ((size_t) index >= list->offset) return 0;
	return (jlong) &list->items[index];
}

jlong repolistGetInstalled(JNIEnv* env, jclass cls, jlong listPtr) {
	repolist_t* list = (repolist_t*) listPtr;
	return (jlong) &list->installed;
}

jlong repolistGetPkg(JNIEnv* env, jclass cls, jlong listPtr, jstring name) {
	repolist_t* list = (repolist_t*) listPtr;
	const char* c_name = (*env)->GetStringUTFChars(env, name, NULL);
	pkg_t* pkg = repolist_get_pkg(list, c_name);
	(*env)->ReleaseStringUTFChars(env, name, c_name);
	return (jlong) pkg;
}

jlong repolistSearchPkg(JNIEnv* env, jclass cls, jlong listPtr, jstring query, jlong position, jlong maximum) {
	
	repolist_t* list = (repolist_t*) listPtr;
	
	const char* c_query = NULL;
	
	if (query != NULL) {
		c_query = (*env)->GetStringUTFChars(env, query, NULL);
	}
	
	pkgs_t* results = calloc(1, sizeof(pkgs_t));
	pkgs_paging_t paging;
	paging.position = (size_t) position;
	paging.maximum = (size_t) maximum;
	
	repolist_search_pkg(list, c_query, paging, results);
	
	if (c_query != NULL) {
		(*env)->ReleaseStringUTFChars(env, query, c_query);
	}
	
	return (jlong) results;
	
}

jlong repolistGetPkgRepo(JNIEnv* env, jclass cls, jlong listPtr, jlong pkgPtr) {
	repolist_t* list = (repolist_t*) listPtr;
	pkg_t* pkg = (pkg_t*) pkgPtr;
	return (jlong) repolist_get_pkg_repo(list, pkg);
}

jint repolistResolveDeps(JNIEnv* env, jclass cls, jlong listPtr, jlong pkgPtr) {
	repolist_t* list = (repolist_t*) listPtr;
	pkg_t* pkg = (pkg_t*) pkgPtr;
	return (jint) repolist_resolve_deps(list, pkg);
}

// ==================== Repository field accessors ====================

jint repoGetType(JNIEnv* env, jclass cls, jlong repoPtr) {
	return (jint) ((repo_t*) repoPtr)->type;
}

jstring repoGetName(JNIEnv* env, jclass cls, jlong repoPtr) {
	return string_from_c(env, ((repo_t*) repoPtr)->name);
}

jstring repoGetRelease(JNIEnv* env, jclass cls, jlong repoPtr) {
	return string_from_c(env, ((repo_t*) repoPtr)->release);
}

jstring repoGetResource(JNIEnv* env, jclass cls, jlong repoPtr) {
	return string_from_c(env, ((repo_t*) repoPtr)->resource);
}

jstring repoGetPlatform(JNIEnv* env, jclass cls, jlong repoPtr) {
	return string_from_c(env, ((repo_t*) repoPtr)->platform);
}

jstring repoGetLocation(JNIEnv* env, jclass cls, jlong repoPtr) {
	return string_from_c(env, ((repo_t*) repoPtr)->location);
}

jstring repoGetSpecification(JNIEnv* env, jclass cls, jlong repoPtr) {
	return string_from_c(env, ((repo_t*) repoPtr)->specification);
}

jint repoGetArchitecture(JNIEnv* env, jclass cls, jlong repoPtr) {
	return (jint) ((repo_t*) repoPtr)->architecture;
}

jstring repoGetBaseUri(JNIEnv* env, jclass cls, jlong repoPtr) {
	base_uri_t* uri = repo_get_uri((repo_t*) repoPtr);
	if (uri == NULL) return NULL;
	return string_from_c(env, uri->value);
}

jlong repoGetPkgs(JNIEnv* env, jclass cls, jlong repoPtr) {
	return (jlong) &((repo_t*) repoPtr)->pkgs;
}

// ==================== Package field accessors ====================

jstring pkgGetName(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return string_from_c(env, ((pkg_t*) pkgPtr)->name);
}

jstring pkgGetVersion(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return string_from_c(env, ((pkg_t*) pkgPtr)->version);
}

jstring pkgGetDescription(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return string_from_c(env, ((pkg_t*) pkgPtr)->description);
}

jstring pkgGetHomepage(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return string_from_c(env, ((pkg_t*) pkgPtr)->homepage);
}

jstring pkgGetBugs(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return string_from_c(env, ((pkg_t*) pkgPtr)->bugs);
}

jstring pkgGetFilename(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return string_from_c(env, ((pkg_t*) pkgPtr)->filename);
}

jlong pkgGetSize(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return (jlong) ((pkg_t*) pkgPtr)->size;
}

jlong pkgGetInstalledSize(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return (jlong) ((pkg_t*) pkgPtr)->installed_size;
}

jint pkgGetArchitecture2(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return (jint) ((pkg_t*) pkgPtr)->arch;
}

jboolean pkgGetObsolete(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return ((pkg_t*) pkgPtr)->obsolete ? JNI_TRUE : JNI_FALSE;
}

jboolean pkgGetInstalled(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return ((pkg_t*) pkgPtr)->installed ? JNI_TRUE : JNI_FALSE;
}

jboolean pkgGetUpgradable(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return ((pkg_t*) pkgPtr)->upgradable ? JNI_TRUE : JNI_FALSE;
}

jboolean pkgGetRemovable(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return ((pkg_t*) pkgPtr)->removable ? JNI_TRUE : JNI_FALSE;
}

jboolean pkgGetAutoinstall(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return ((pkg_t*) pkgPtr)->autoinstall ? JNI_TRUE : JNI_FALSE;
}

jlong pkgGetDepends(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return (jlong) ((pkg_t*) pkgPtr)->depends;
}

jlong pkgGetRecommends(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return (jlong) ((pkg_t*) pkgPtr)->recommends;
}

jlong pkgGetSuggests(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return (jlong) ((pkg_t*) pkgPtr)->suggests;
}

jlong pkgGetBreaks(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return (jlong) ((pkg_t*) pkgPtr)->breaks;
}

jlong pkgGetReplaces(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return (jlong) ((pkg_t*) pkgPtr)->replaces;
}

jstring pkgGetProvides(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return string_from_c(env, (char*) ((pkg_t*) pkgPtr)->provides);
}

jlong pkgGetMaintainers(JNIEnv* env, jclass cls, jlong pkgPtr) {
	pkg_t* pkg = (pkg_t*) pkgPtr;
	if (!pkg->resolved) return 0;
	return (jlong) pkg->maintainer;
}

jlong pkgGetRepo(JNIEnv* env, jclass cls, jlong pkgPtr) {
	return (jlong) ((pkg_t*) pkgPtr)->repo;
}

// ==================== Packages (pkgs_t) access ====================

jlong pkgsGetSize(JNIEnv* env, jclass cls, jlong pkgsPtr) {
	if (pkgsPtr == 0) return 0;
	pkgs_t* pkgs = (pkgs_t*) pkgsPtr;
	return (jlong) pkgs->offset;
}

jlong pkgsGetItem(JNIEnv* env, jclass cls, jlong pkgsPtr, jlong index) {
	if (pkgsPtr == 0) return 0;
	pkgs_t* pkgs = (pkgs_t*) pkgsPtr;
	if ((size_t) index >= pkgs->offset) return 0;
	return (jlong) pkgs->items[index];
}

void pkgsFree(JNIEnv* env, jclass cls, jlong pkgsPtr, jint copy) {
	if (pkgsPtr == 0) return;
	pkgs_t* pkgs = (pkgs_t*) pkgsPtr;
	pkgs_free(pkgs, (int) copy);
	free(pkgs);
}

// ==================== Maintainers access ====================

jlong maintainersGetSize(JNIEnv* env, jclass cls, jlong maintainersPtr) {
	if (maintainersPtr == 0) return 0;
	maintainers_t* m = (maintainers_t*) maintainersPtr;
	return (jlong) m->offset;
}

jstring maintainersGetName(JNIEnv* env, jclass cls, jlong maintainersPtr, jlong index) {
	if (maintainersPtr == 0) return NULL;
	maintainers_t* m = (maintainers_t*) maintainersPtr;
	if ((size_t) index >= m->offset) return NULL;
	return string_from_c(env, m->items[index].name);
}

jstring maintainersGetEmail(JNIEnv* env, jclass cls, jlong maintainersPtr, jlong index) {
	if (maintainersPtr == 0) return NULL;
	maintainers_t* m = (maintainers_t*) maintainersPtr;
	if ((size_t) index >= m->offset) return NULL;
	return string_from_c(env, m->items[index].email);
}

// ==================== Operations ====================

jint repolistInstallPackage(JNIEnv* env, jclass cls, jlong listPtr, jobjectArray packages) {
	repolist_t* list = (repolist_t*) listPtr;
	jsize count = (*env)->GetArrayLength(env, packages);
	char** c_pkgs = calloc((size_t)(count + 1), sizeof(char*));
	if (c_pkgs == NULL) return -27;
	for (jsize i = 0; i < count; i++) {
		jstring js = (jstring) (*env)->GetObjectArrayElement(env, packages, i);
		if (js != NULL) {
			c_pkgs[i] = (char*) (*env)->GetStringUTFChars(env, js, NULL);
		}
	}
	c_pkgs[count] = NULL;
	jint result = (jint) repolist_install_package(list, c_pkgs);
	for (jsize i = 0; i < count; i++) {
		if (c_pkgs[i] != NULL) {
			jstring js = (jstring) (*env)->GetObjectArrayElement(env, packages, i);
			(*env)->ReleaseStringUTFChars(env, js, c_pkgs[i]);
		}
	}
	free(c_pkgs);
	return result;
}

jint repolistRemovePackage(JNIEnv* env, jclass cls, jlong listPtr, jobjectArray packages) {
	repolist_t* list = (repolist_t*) listPtr;
	jsize count = (*env)->GetArrayLength(env, packages);
	char** c_pkgs = calloc((size_t)(count + 1), sizeof(char*));
	if (c_pkgs == NULL) return -27;
	for (jsize i = 0; i < count; i++) {
		jstring js = (jstring) (*env)->GetObjectArrayElement(env, packages, i);
		if (js != NULL) {
			c_pkgs[i] = (char*) (*env)->GetStringUTFChars(env, js, NULL);
		}
	}
	c_pkgs[count] = NULL;
	jint result = (jint) repolist_remove_package(list, c_pkgs);
	for (jsize i = 0; i < count; i++) {
		if (c_pkgs[i] != NULL) {
			jstring js = (jstring) (*env)->GetObjectArrayElement(env, packages, i);
			(*env)->ReleaseStringUTFChars(env, js, c_pkgs[i]);
		}
	}
	free(c_pkgs);
	return result;
}

jint optionsLoad(JNIEnv* env, jclass cls, jstring directory) {
	const char* c_dir = (*env)->GetStringUTFChars(env, directory, NULL);
	jint result = (jint) options_load(c_dir);
	(*env)->ReleaseStringUTFChars(env, directory, c_dir);
	return result;
}

jint getNproc(JNIEnv* env, jclass cls) {
	return (jint) get_nproc();
}

jstring osdetectGetPlatform(JNIEnv* env, jclass cls) {
	const char* platform = osdetect_getplatform();
	return string_from_c(env, (char*) platform);
}

// ==================== Architecture helpers ====================

jstring archUnstringify(JNIEnv* env, jclass cls, jint arch) {
	const char* name = repoarch_unstringify((architecture_t) arch);
	return string_from_c(env, (char*) name);
}

jint getArchitecture2(JNIEnv* env, jclass cls, jstring name) {
	const char* c_name = (*env)->GetStringUTFChars(env, name, NULL);
	jint result = (jint) get_architecture(c_name);
	(*env)->ReleaseStringUTFChars(env, name, c_name);
	return result;
}

// ==================== Native method registration ====================

static JNINativeMethod JNI_NATIVES_METHODS[] = {
	// Config
	{"repoSetConfigDir",	 "(Ljava/lang/String;)I",					 (void*)repoSetConfigDir},
	{"repoGetConfigDir",	 "()Ljava/lang/String;",					 (void*)repoGetConfigDir},
	// RepoList lifecycle
	{"repolistCreate",	   "()J",									   (void*)repolistCreate},
	{"repolistFree",		 "(J)V",									  (void*)repolistFree},
	{"repolistLoad",		 "(J)I",									  (void*)repolistLoad},
	{"repolistDestroy",	  "(J)I",									  (void*)repolistDestroy},
	// RepoList queries
	{"repolistGetSize",	  "(J)J",									  (void*)repolistGetSize},
	{"repolistGetRepo",	  "(JJ)J",									 (void*)repolistGetRepo},
	{"repolistGetInstalled", "(J)J",									  (void*)repolistGetInstalled},
	{"repolistGetPkg",	   "(JLjava/lang/String;)J",					(void*)repolistGetPkg},
	{"repolistSearchPkg",	"(JLjava/lang/String;JJ)J",				  (void*)repolistSearchPkg},
	{"repolistGetPkgRepo",   "(JJ)J",									 (void*)repolistGetPkgRepo},
	{"repolistResolveDeps",  "(JJ)I",									 (void*)repolistResolveDeps},
	// Repository field accessors
	{"repoGetType",		  "(J)I",									  (void*)repoGetType},
	{"repoGetName",		  "(J)Ljava/lang/String;",					 (void*)repoGetName},
	{"repoGetRelease",	   "(J)Ljava/lang/String;",					 (void*)repoGetRelease},
	{"repoGetResource",	  "(J)Ljava/lang/String;",					 (void*)repoGetResource},
	{"repoGetPlatform",	  "(J)Ljava/lang/String;",					 (void*)repoGetPlatform},
	{"repoGetLocation",	  "(J)Ljava/lang/String;",					 (void*)repoGetLocation},
	{"repoGetSpecification", "(J)Ljava/lang/String;",					 (void*)repoGetSpecification},
	{"repoGetArchitecture",  "(J)I",									  (void*)repoGetArchitecture},
	{"repoGetPkgs",		  "(J)J",									  (void*)repoGetPkgs},
	{"repoGetBaseUri",	   "(J)Ljava/lang/String;",					 (void*)repoGetBaseUri},
	// Package field accessors
	{"pkgGetName",		   "(J)Ljava/lang/String;",					 (void*)pkgGetName},
	{"pkgGetVersion",		"(J)Ljava/lang/String;",					 (void*)pkgGetVersion},
	{"pkgGetDescription",	"(J)Ljava/lang/String;",					 (void*)pkgGetDescription},
	{"pkgGetHomepage",	   "(J)Ljava/lang/String;",					 (void*)pkgGetHomepage},
	{"pkgGetBugs",		   "(J)Ljava/lang/String;",					 (void*)pkgGetBugs},
	{"pkgGetFilename",	   "(J)Ljava/lang/String;",					 (void*)pkgGetFilename},
	{"pkgGetSize",		   "(J)J",									  (void*)pkgGetSize},
	{"pkgGetInstalledSize",  "(J)J",									  (void*)pkgGetInstalledSize},
	{"pkgGetArchitecture",   "(J)I",									  (void*)pkgGetArchitecture2},
	{"pkgGetObsolete",	   "(J)Z",									  (void*)pkgGetObsolete},
	{"pkgGetInstalled",	  "(J)Z",									  (void*)pkgGetInstalled},
	{"pkgGetUpgradable",	 "(J)Z",									  (void*)pkgGetUpgradable},
	{"pkgGetRemovable",	  "(J)Z",									  (void*)pkgGetRemovable},
	{"pkgGetAutoinstall",	"(J)Z",									  (void*)pkgGetAutoinstall},
	{"pkgGetDepends",		"(J)J",									  (void*)pkgGetDepends},
	{"pkgGetRecommends",	 "(J)J",									  (void*)pkgGetRecommends},
	{"pkgGetSuggests",	   "(J)J",									  (void*)pkgGetSuggests},
	{"pkgGetBreaks",		 "(J)J",									  (void*)pkgGetBreaks},
	{"pkgGetReplaces",	   "(J)J",									  (void*)pkgGetReplaces},
	{"pkgGetProvides",	   "(J)Ljava/lang/String;",					 (void*)pkgGetProvides},
	{"pkgGetMaintainers",	"(J)J",									  (void*)pkgGetMaintainers},
	{"pkgGetRepo",		   "(J)J",									  (void*)pkgGetRepo},
	// Packages (pkgs_t) access
	{"pkgsGetSize",		  "(J)J",									  (void*)pkgsGetSize},
	{"pkgsGetItem",		  "(JJ)J",									 (void*)pkgsGetItem},
	{"pkgsFree",			 "(JI)V",									 (void*)pkgsFree},
	// Maintainers access
	{"maintainersGetSize",   "(J)J",									  (void*)maintainersGetSize},
	{"maintainersGetName",   "(JJ)Ljava/lang/String;",					(void*)maintainersGetName},
	{"maintainersGetEmail",  "(JJ)Ljava/lang/String;",					(void*)maintainersGetEmail},
	// Operations
	{"repolistInstallPackage",  "(J[Ljava/lang/String;)I",			   (void*)repolistInstallPackage},
	{"repolistRemovePackage",   "(J[Ljava/lang/String;)I",			   (void*)repolistRemovePackage},
	{"optionsLoad",			 "(Ljava/lang/String;)I",				  (void*)optionsLoad},
	{"getNproc",				"()I",									(void*)getNproc},
	{"osdetectGetPlatform",	 "()Ljava/lang/String;",				   (void*)osdetectGetPlatform},
	// Architecture helpers
	{"archUnstringify",	  "(I)Ljava/lang/String;",					 (void*)archUnstringify},
	{"getArchitecture",	  "(Ljava/lang/String;)I",					 (void*)getArchitecture2},
};

jint JNI_OnLoad_repository(JavaVM* vm, JNIEnv* env) {
	jclass cls = (*env)->FindClass(env, "com/amanoteam/nz/library/Repository");
	if (cls == NULL) {
		return JNI_ERR;
	}
	if ((*env)->RegisterNatives(env, cls, JNI_NATIVES_METHODS, sizeof(JNI_NATIVES_METHODS) / sizeof(*JNI_NATIVES_METHODS)) < 0) {
		return JNI_ERR;
	}
	return JNI_OK;
}
