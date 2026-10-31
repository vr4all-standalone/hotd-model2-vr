plugins { id("com.android.application") }
android {
    namespace = "com.vrlightgunhub.hotdmodel2"
    ndkVersion = "29.0.14206865"
    compileSdk { version = release(36) { minorApiLevel = 1 } }
    defaultConfig {
        applicationId = "com.vrlightgunhub.hotdmodel2"
        minSdk = 29
        targetSdk = 36
        versionCode = 8
        versionName = "0.1.7-beta"
        ndk { abiFilters += "arm64-v8a" }
    }
    signingConfigs { create("local") {
        storeFile = rootProject.file("../private/hotd-model2.keystore")
        storePassword = "android"
        keyAlias = "hotdmodel2"
        keyPassword = "android"
    } }
    buildTypes { getByName("release") { signingConfig = signingConfigs.getByName("local"); isMinifyEnabled = false } }
    sourceSets.getByName("main").assets.srcDirs("../../assets")
    sourceSets.getByName("main").jniLibs.srcDirs(providers.gradleProperty("hotdModel2NativeDir").orNull ?: rootProject.file("../build/quest/jniLibs"))
    packaging { jniLibs { useLegacyPackaging = true } }
}

layout.buildDirectory.set(rootProject.layout.projectDirectory.dir("../build/android/app"))
