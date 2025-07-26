#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <android/log.h>

#include "zygisk/module.h"
#include "zygisk/zygisk.hpp"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

//#define DEBUG

/*
PUBG Mobile (Global Version)
Package Name: com.tencent.ig

PUBG Mobile Korea (KR Version)
Package Name: com.pubg.krmobile

PUBG Mobile Lite
Package Name: com.tencent.iglite

Battlegrounds Mobile India (BGMI)
Package Name: com.pubg.imobile
*/

struct SpoofConfig {
    std::vector<std::string> packages;
    const char *model;
    const char *product;
    const char *device;
    const char *brand;
    const char *manufacturer;
    const char *fingerprint;
};

static const std::vector<SpoofConfig> spoofConfigs = {
        // More configs can be added later for other android games
        {
                // Spoofing pubg variants
                {"com.tencent.ig", "com.pubg.krmobile", "com.tencent.iglite", "com.pubg.imobile"},
                "CPH2649",    // Model
                "CPH2649IN",  // Product
                "CPH2649IN",  // Device
                "OnePlus",    // Brand
                "OnePlus",    // Manufacturer
                // Fingerprint
                "OnePlus/CPH2649IN/OP5D55L1:16/BP22.250124.009/V.R4T3.38870dc-1b79a4a-1b79a4c:user/release-keys"
        }
};

char package_name[256] = {0};
static int spoof_type = -1;

class unlockfps : public zygisk::ModuleBase {
private:
    Api *api;
    JNIEnv *env;

    void
    injectBuild(const char *model1, const char *product1, const char *device1, const char *brand1,
                const char *manufacturer1, const char *finger1) {
        if (env == nullptr) {
            LOGW("failed to inject android.os.Build for %s due to env is null", package_name);
            return;
        }

        jclass build_class = env->FindClass("android/os/Build");
        if (build_class == nullptr) {
#ifndef  DEBUG
            LOGW("failed to inject android.os.Build for %s due to build is null", package_name);
#else
            LOGI("inject android.os.Build for %s with \nPRODUCT:%s \nMODEL:%s \nDEVICE:%s \nBRAND:%s \nMANUFACTURER:%s \nFINGERPRINT:%s",
                 package_name, product1, model1, device1, brand1, manufacturer1, finger1);
#endif
            return;
        }

        jstring product = env->NewStringUTF(product1);
        jstring model = env->NewStringUTF(model1);
        jstring device = env->NewStringUTF(device1);
        jstring brand = env->NewStringUTF(brand1);
        jstring manufacturer = env->NewStringUTF(manufacturer1);
        jstring finger = env->NewStringUTF(finger1);

        auto setStaticField = [&](const char *fieldName, jstring value) {
            jfieldID fieldID = env->GetStaticFieldID(build_class, fieldName, "Ljava/lang/String;");
            if (fieldID != nullptr) {
                env->SetStaticObjectField(build_class, fieldID, value);
            }
        };

        setStaticField("PRODUCT", product);
        setStaticField("MODEL", model);
        setStaticField("DEVICE", device);
        setStaticField("BRAND", brand);
        setStaticField("MANUFACTURER", manufacturer);
        if (strlen(finger1) > 0)
            setStaticField("FINGERPRINT", finger);

        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }

        env->DeleteLocalRef(product);
        env->DeleteLocalRef(model);
        env->DeleteLocalRef(device);
        env->DeleteLocalRef(brand);
        env->DeleteLocalRef(manufacturer);
        env->DeleteLocalRef(finger);
    }

    static int getSpoof(const char *process) {
        for (size_t i = 0; i < spoofConfigs.size(); ++i) {
            for (const auto &pkg: spoofConfigs[i].packages) {
                if (strstr(process, pkg.c_str())) {
                    LOGI("Spoof trigger found for package: %s", process);
                    return static_cast<int>(i);
                }
            }
        }
#ifdef DEBUG
        LOGERRNO("Spoof cannot be triggered for package: %s", process);
#endif
        return -1;
    }

public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        // Use JNI to fetch our process name
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        spoof_type = getSpoof(process);
        strncpy(package_name, process, sizeof(package_name) - 1);
        package_name[sizeof(package_name) - 1] = '\0';
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

    // Pick values dynamically from the matching config struct
    void postAppSpecialize(const AppSpecializeArgs *) override {
        if (spoof_type >= 0) {
            const auto &config = spoofConfigs[spoof_type];
            injectBuild(config.model, config.product, config.device, config.brand,
                        config.manufacturer, config.fingerprint);
        }
    }
};

REGISTER_ZYGISK_MODULE(unlockfps)