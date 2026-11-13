# PPSSPP Realme C21Y Release R8 / ProGuard keep rules.
# Preserve all Java classes and methods referenced via JNI or Android callbacks.

-keep class org.ppsspp.ppsspp.** { *; }
-keepclassmembers class org.ppsspp.ppsspp.** { *; }

# Keep generated Tombstone protobuf classes for ApplicationExitInfo crash inspection.
-keep class org.ppsspp.proto.** { *; }

# Keep native JNI declarations on all classes.
-keepclasseswithmembernames,includedescriptorclasses class * {
    native <methods>;
}
