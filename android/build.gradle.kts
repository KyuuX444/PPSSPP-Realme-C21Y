plugins {
	id("com.android.application")
	id("com.google.protobuf")
}

abstract class PrepareTrimmedAssetsTask @javax.inject.Inject constructor(
	private val fs: org.gradle.api.file.FileSystemOperations
) : org.gradle.api.DefaultTask() {
	@get:org.gradle.api.tasks.InputDirectory
	@get:org.gradle.api.tasks.PathSensitive(org.gradle.api.tasks.PathSensitivity.RELATIVE)
	abstract val sourceDir: org.gradle.api.file.DirectoryProperty

	@get:org.gradle.api.tasks.OutputDirectory
	abstract val outputDir: org.gradle.api.file.DirectoryProperty

	@org.gradle.api.tasks.TaskAction
	fun syncAssets() {
		fs.sync {
			from(sourceDir)
			into(outputDir)
			exclude("compatvr.ini", "redump.csv", "lang/README.md")
			exclude { details ->
				details.relativePath.pathString.startsWith("lang/") &&
					details.name.endsWith(".ini") &&
					details.name != "en_US.ini" &&
					details.name != "id_ID.ini"
			}
		}
	}
}

// Utility function used to get the git version
// All the below replaces the following "gladed.androidgitversion" config:
// configure<com.gladed.androidgitversion.AndroidGitVersionExtension> {
//   codeFormat = "MNNPPBBBB"
//   format = "%tag%%-count%%-branch%%-dirty%"
//   prefix = "v"  // Only tags beginning with v are considered.
//   untrackedIsDirty = false
// }

fun ProviderFactory.git(vararg args: String): String {
	val result = exec {
		commandLine("git", *args)
		isIgnoreExitValue = true
	}
	return result.standardOutput.asText.get().trim()
}

val gitTag = providers.git("describe", "--tags", "--match", "v*", "--abbrev=0")
	.ifEmpty { "v0.0.0" }
val commitsSinceTag = providers.git(
	"rev-list", "$gitTag..HEAD", "--count"
).toIntOrNull() ?: 0
val branchName = providers.git(
	"rev-parse", "--abbrev-ref", "HEAD"
).replace("/", "-")
val isDirty = providers.git("status", "--porcelain")
	.lineSequence()
	.any { it.isNotBlank() && !it.startsWith("??") } // untrackedIsDirty = false

val versionParts = gitTag
	.removePrefix("v")
	.split(".")
	.mapNotNull { it.toIntOrNull() }

// Ensure we have at least 3 components by padding with 0
val major = versionParts.getOrElse(0) { 0 }
val minor = versionParts.getOrElse(1) { 0 }
val patch = versionParts.getOrElse(2) { 0 }

val gitVersionName = if (commitsSinceTag == 0) gitTag else buildString {
	append(gitTag)
	append("-")
	append(commitsSinceTag)
	append("-")
	append(branchName)
	if (isDirty) append("-dirty")
}

val gitVersionCode =
	major * 100_000_000 +
		minor * 1000_000 +
		patch * 10_000 +
		commitsSinceTag

dependencies {
	// appcompat 1.8.0 and later need minSdk 23.
	implementation("androidx.appcompat:appcompat:1.7.1")
	implementation("androidx.documentfile:documentfile:1.1.0")
	// Keep the runtime and protoc versions in step.
	implementation("com.google.protobuf:protobuf-javalite:4.36.2")
	// Leak detection for debug builds, opt-in since its heap dumps freeze the app: ./gradlew -PleakCanary ...
	if (project.hasProperty("leakCanary")) {
		debugImplementation("com.squareup.leakcanary:leakcanary-android:2.14")
	}
}

protobuf {
	protoc {
		artifact = "com.google.protobuf:protoc:4.36.2"
	}

	generateProtoTasks {
		all().forEach { task ->
			task.builtins {
				register("java") {
					option("lite")
				}
			}
		}
	}
}

android {
	flavorDimensions.add("variant")
	namespace = "org.ppsspp.ppsspp"
	signingConfigs {
		getByName("debug") {
			storeFile = file("debug.keystore")
		}
		create("optimized") {
			storeFile = file("debug.keystore")
		}
		if (project.hasProperty("RELEASE_STORE_FILE")) {
			create("release") {
				storeFile = file(project.property("RELEASE_STORE_FILE") as String)
				storePassword = project.property("RELEASE_STORE_PASSWORD") as String
				keyAlias = project.property("RELEASE_KEY_ALIAS") as String
				keyPassword = project.property("RELEASE_KEY_PASSWORD") as String
			}
		}
	}

	compileSdk = 37
	ndkVersion = "29.0.14206865"

	compileOptions {
		sourceCompatibility = JavaVersion.VERSION_17
		targetCompatibility = JavaVersion.VERSION_17
	}

	lint {
		baseline = file("lint-baseline.xml")
	}

	defaultConfig {
		applicationId = "org.ppsspp.ppsspp"
		// Access the git version info via the extension
		if (gitVersionName != "unknown") {
			println("INFO: Overriding Android Version Name, Code: $gitVersionName $gitVersionCode")
			versionName = gitVersionName
			versionCode = gitVersionCode
		} else {
			println("(not using these:) Android Version Name, Code: $gitVersionName $gitVersionCode")
		}
		file("versionname.txt").writeText(gitVersionName)
		file("versioncode.txt").writeText(gitVersionCode.toString())

		minSdk = 21
		targetSdk = 37
		if (project.hasProperty("ANDROID_VERSION_CODE") && project.hasProperty("ANDROID_VERSION_NAME")) {
			versionCode = (project.property("ANDROID_VERSION_CODE") as String).toInt()
			versionName = project.property("ANDROID_VERSION_NAME") as String
		}
	}
	buildTypes {
		getByName("debug") {
			isDefault = true
			isMinifyEnabled = false
			isJniDebuggable = true
			signingConfig = signingConfigs.getByName("debug")
		}
		create("optimized") {
			isMinifyEnabled = false
			isJniDebuggable = true
			signingConfig = signingConfigs.getByName("debug")
		}
		getByName("release") {
			isMinifyEnabled = true
			isShrinkResources = true
			proguardFiles(
				getDefaultProguardFile("proguard-android-optimize.txt"),
				"proguard-rules.pro"
			)
			if (project.hasProperty("RELEASE_STORE_FILE")) {
				println("RELEASE_STORE_FILE is set. Using as signing config.")
				signingConfig = signingConfigs.getByName("release")
			} else {
				println("WARNING: RELEASE_STORE_FILE is missing. Release builds will be unusable.")
			}
		}
	}
	externalNativeBuild {
		cmake {
			path = file("../CMakeLists.txt")
		}
	}
	packaging {
		jniLibs.useLegacyPackaging = true
	}
	sourceSets {
		getByName("main") {
			manifest.srcFile("AndroidManifest.xml")
			res.directories.add("res")
			java.directories.add("src")
			aidl.directories.add("src")
			resources.directories.add("src")
		}
		create("normal") {
			res.directories.add("normal/res")
			assets.directories.add("../assets")
		}
		create("gold") {
			res.directories.add("gold/res")
			assets.directories.add("../assets")
		}
		create("vr") {
			res.directories.add("normal/res")
			assets.directories.add("../assets")
			manifest.srcFile("VRManifest.xml")
		}
		create("legacy") {
			res.directories.add("legacy/res")
			assets.directories.add("../assets")
		}
		create("realmeC21y") {
			res.directories.add("normal/res")
			manifest.srcFile("realmeC21y/AndroidManifest.xml")
		}
	}
	productFlavors {
		create("normal") {
			isDefault = true
			applicationId = "org.ppsspp.ppsspp"
			dimension = "variant"
			externalNativeBuild {
				cmake {
					// **FIXED**: Using simple "listOf"
					arguments.addAll(listOf(
						"-DANDROID=true",
						"-DANDROID_PLATFORM=android-21",
						"-DANDROID_TOOLCHAIN=clang",
						"-DANDROID_CPP_FEATURES=",
						"-DANDROID_STL=c++_shared"
					))
				}
			}
			ndk {
				abiFilters.addAll(listOf("armeabi-v7a", "arm64-v8a", "x86_64"))
				// debugSymbolLevel = "FULL"  // These don't actually help much in the Google Play crash report. We do still have symbols locally.
			}
		}
		create("gold") {
			applicationId = "org.ppsspp.ppssppgold"
			dimension = "variant"
			externalNativeBuild {
				cmake {
					arguments.addAll(listOf(
						"-DANDROID=true",
						"-DANDROID_PLATFORM=android-21",
						"-DANDROID_TOOLCHAIN=clang",
						"-DANDROID_CPP_FEATURES=",
						"-DANDROID_STL=c++_shared",
						"-DGOLD=TRUE"
					))
				}
			}
			ndk {
				abiFilters.addAll(listOf("armeabi-v7a", "arm64-v8a", "x86_64"))
				// debugSymbolLevel = "FULL"
			}
		}
		create("legacy") {
			applicationId = "org.ppsspp.ppsspplegacy"
			dimension = "variant"
			targetSdk = 29  // To avoid scoped storage, which is the point of the legacy APK
			externalNativeBuild {
				cmake {
					arguments.addAll(listOf(
						"-DANDROID=true",
						"-DANDROID_PLATFORM=android-21",
						"-DANDROID_TOOLCHAIN=clang",
						"-DANDROID_CPP_FEATURES=",
						"-DANDROID_STL=c++_shared",
						"-DANDROID_LEGACY=TRUE"
					))
				}
			}
			ndk {
				abiFilters.addAll(listOf("armeabi-v7a", "arm64-v8a"))
				// debugSymbolLevel = "FULL"
			}
		}
		create("vr") {
			applicationId = "org.ppsspp.ppssppvr"
			dimension = "variant"
			targetSdk = 29  // To avoid scoped storage, which doesn't work properly on Oculus
			externalNativeBuild {
				cmake {
					arguments.addAll(listOf(
						"-DANDROID=true",
						"-DANDROID_PLATFORM=android-21",
						"-DANDROID_TOOLCHAIN=clang",
						"-DANDROID_CPP_FEATURES=",
						"-DANDROID_STL=c++_shared",
						"-DOPENXR=TRUE",
						"-DANDROID_LEGACY=TRUE"
					))
				}
			}
			ndk {
				abiFilters.addAll(listOf("arm64-v8a"))
			}
		}
		create("realmeC21y") {
			applicationId = "org.ppsspp.ppsspp"
			versionNameSuffix = "-realme-c21y"
			dimension = "variant"
			externalNativeBuild {
				cmake {
					arguments.addAll(listOf(
						"-DANDROID=true",
						"-DANDROID_PLATFORM=android-21",
						"-DANDROID_TOOLCHAIN=clang",
						"-DANDROID_CPP_FEATURES=",
						"-DANDROID_STL=c++_shared",
						"-DREALME_C21Y_TUNE=ON",
						"-DUSE_THIN_LTO=ON",
						"-DUSE_DISCORD=OFF",
						"-DUSE_ADRENOTOOLS=OFF",
						"-DHEADLESS=OFF",
						"-DUNITTEST=OFF"
					))
				}
			}
			ndk {
				abiFilters.add("arm64-v8a")
				debugSymbolLevel = "NONE"
			}
		}
	}
	buildFeatures {
		aidl = true
		buildConfig = true
	}
}
androidComponents {
	beforeVariants(selector().all()) { variantBuilder ->
		// **FIXED**: Using simple "setOf"
		val enabledVariants = setOf(
			"normalDebug", "normalOptimized", "normalRelease",
			"goldDebug", "goldRelease",
			"vrDebug", "vrOptimized", "vrRelease",
			"legacyDebug", "legacyOptimized", "legacyRelease",
			"realmeC21yDebug", "realmeC21yOptimized", "realmeC21yRelease"
		)
		variantBuilder.enable = variantBuilder.name in enabledVariants
	}
	onVariants(selector().withFlavor("variant" to "realmeC21y")) { variant ->
		val capName = variant.name.replaceFirstChar { it.uppercase() }
		val assetTask = tasks.register<PrepareTrimmedAssetsTask>("prepare${capName}Assets") {
			sourceDir.set(layout.projectDirectory.dir("../assets"))
		}
		variant.sources.assets?.addGeneratedSourceDirectory(
			assetTask,
			PrepareTrimmedAssetsTask::outputDir
		)
	}
}

/*
afterEvaluate {
	android.sourceSets.getByName("main").assets.srcDirs.forEach {
		println(it)
	}
}*/

// Use the below to get a lot of deprecation warnings.
// Not really useful to fix most of them though, as the old paths are often needed to support
// old Android versions.
/*
tasks.withType<JavaCompile> {
	options.compilerArgs.addAll(listOf("-Xlint:deprecation"))
}
*/
