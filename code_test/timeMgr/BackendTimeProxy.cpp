#include "BackendTimeProxy.h"
#include "EventManager.h"
#include "UtilTMGR.h"

#include <nf/Logging.h>
#include <nf/Executor.h>
#include <iostream>
#include <fcntl.h>		 	/* File control options */
#include <sys/stat.h>    	/* Data return by stat() function */
#include <sys/shm.h>     	/* Share memory facility */
#include <sys/mman.h>
#include <unordered_set>
#include <fstream>
#include <openssl/ssl.h>
#include <openssl/ocsp.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/tls1.h>
#include <filesystem>
#include "CurlCallback.h"
	
#include "ITimeMgrType.h"
	
using nf::Timer;
using nf::Executor;
using std::optional;
using namespace std::chrono;
using namespace std;
using namespace UtilTMGR;
	
using TimeMgrType::Proxy;
using TimeMgrType::ProxyTime;
using namespace ProvisioningTimeManager;
	
NF_LOG_DEFINE_USE_CONTEXT(be_time_proxy, TPB, "Backend Time Proxy");
	
std::atomic<uint32_t> BackendTimeProxy::failedConnections{0}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
	
BackendTimeProxy::BackendTimeProxy(EventManager *em)
    : manager(em)
    , delay(BackendTimeProxy::DelayStartup)
    , delayMaintain(BackendTimeProxy::DelayCounting)
    , timerID(std::this_thread::get_id())
    , timeInfo(Proxy::BE)
    , mSPVerRVP(UtilTMGR::getSPVerRVP())
{
    nf::info(ActiveLogContext, "--Start-- BackendTimeProxy");
    timer = Timer::construct(delay, [this]() {
        if (failedConnections >= MAX_RETRY_FALLBACK_FAILED) {
            nf::info(ActiveLogContext, "Reset failed connections to try get BE time again");
            failedConnections = 0;
        }

        tryGet();
        return false;
    });

    timer->start();

    timerMaintain = Timer::construct(delayMaintain, [this]() {
        maintainTime += 1;
        return true;
    });

    timeWrapper = std::make_unique<TimeWrapper>();
    fileSystemWrapper = std::make_unique<FileSystemWrapper>();
    registerSomeIPProvider();
}
	
BackendTimeProxy::~BackendTimeProxy() {
    unregisterSomeIPProvider();
    mThread.syncJoin();
}
	
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    if (contents != nullptr && userp != nullptr) {
        if(strcasestr(static_cast<char*>(contents), "X-HTTPSTIME")) {
            (static_cast<std::string*>(userp))->append(static_cast<char*>(contents), size * nmemb);
        }
    }

    return size * nmemb;
}
	
// Debug callback function
static int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr) {
    (void)handle; // Unused parameter
    (void)userptr; // Unused parameter

    nf::info(ActiveLogContext, "[CURL DEBUG] Type: {} Data: {}", nf::pub(type), nf::pub(std::string(data, size)));

    return 0;
}

int BackendTimeProxy::numericMonthOf(const std::string &monthAbbr)
{
    auto it = Months.find(monthAbbr);

    return it != Months.end() ? it->second : -1;
}
	
bool BackendTimeProxy::isNumeric(const std::string &time) {
    // Check time is digit or not
    return std::all_of(time.begin(), time.end(), [](char c) {
        return (c >= '0') && (c <= '9');
    });
}
	
ProxyTime BackendTimeProxy::fromString(const std::string &timeBuff)
{
    // Parse backend time format: Wed, 29 Jun 2022 09:21:42
    ProxyTime beTime(Proxy::BE);
    std::vector<std::string> tokens;
    std::stringstream dateTime(timeBuff);
    std::string temp;

    try {
        nf::info(ActiveLogContext, "Starting convert to time");
        if(timeBuff.length() >= SLDD_PAYLOAD_LENGTH) {
            // Parse by " " to get: "Wed," "29" "Jun" "2022" "09:21:42"
            while(getline(dateTime, temp, ' ')) {
                tokens.push_back(temp);
            }

            // Parse by "09:21:42" by ":" to get time: "09" "21" "42"
            std::stringstream time(tokens[DATE_TIME_TOKEN_INDEX]);
            while(getline(time, temp, ':')) {
                tokens.push_back(temp);
            }

            do {
                if (numericMonthOf(tokens[DATE_MONTH_INDEX]) == -1) {
                    nf::info(ActiveLogContext, "sldd set BETime is not valid");
                    break;
                }

                // Check time is numeric or not
                if (!isNumeric(tokens[DATE_YEAR_INDEX]) ||
                    !isNumeric(tokens[DATE_DAY_INDEX]) ||
                    !isNumeric(tokens[TIME_HOUR_INDEX]) ||
                    !isNumeric(tokens[TIME_MINUTE_INDEX]) ||
                    !isNumeric(tokens[DATE_SECOND_INDEX])) {
                        nf::info(ActiveLogContext, "sldd set BETime is not valid");
                        break;
                }

                // Check time valid or not
                beTime.date.year    = static_cast<uint16_t>(stoi(tokens[DATE_YEAR_INDEX]));
                beTime.date.month   = static_cast<uint8_t>(numericMonthOf(tokens[DATE_MONTH_INDEX]));
                beTime.date.day     = static_cast<uint8_t>(stoi(tokens[DATE_DAY_INDEX]));
                beTime.time.hour    = static_cast<uint8_t>(stoi(tokens[TIME_HOUR_INDEX]));
                beTime.time.minute  = static_cast<uint8_t>(stoi(tokens[TIME_MINUTE_INDEX]));
                beTime.time.second  = static_cast<uint8_t>(stoi(tokens[DATE_SECOND_INDEX]));

                struct tm timecheck = {};
                timecheck.tm_year   = beTime.date.year - 1900;
                timecheck.tm_mon    = beTime.date.month - 1;
                timecheck.tm_mday   = beTime.date.day;
                timecheck.tm_hour   = beTime.time.hour;
                timecheck.tm_min    = beTime.time.minute;
                timecheck.tm_sec    = beTime.time.second;

                // Using timegm() because it takes the input value to be Coordinated Universal Time (UTC) instead of local time
                (void)timeWrapper->timegm(&timecheck);

                // If time is invalid, timecheck will be changed after using "timegm(&timecheck)". So now compare timecheck with t
                if ((timecheck.tm_year != (beTime.date.year - 1900)) ||
                    (timecheck.tm_mon != (beTime.date.month - 1)) ||
                    (timecheck.tm_mday != beTime.date.day) ||
                    (timecheck.tm_hour != beTime.time.hour) ||
                    (timecheck.tm_min != beTime.time.minute) ||
                    (timecheck.tm_sec != beTime.time.second)) {
                        nf::info(ActiveLogContext, "sldd set BETime is not valid");
                        beTime.date.year = 0;
                }
            } while (false);
        }
    } catch (const std::invalid_argument& iv) {
        nf::error(ActiveLogContext, "Invalid argument catched: {}", nf::pub(iv.what()));
    }

    return beTime;
}
	
const std::unordered_map<std::string, int> BackendTimeProxy::Months = {
    { "Jan", 1 },
    { "Feb", 2 },
    { "Mar", 3 },
    { "Apr", 4 },
    { "May", 5 },
    { "Jun", 6 },
    { "Jul", 7 },
    { "Aug", 8 },
    { "Sep", 9 },
    { "Oct", 10 },
    { "Nov", 11 },
    { "Dec", 12 }
};
	
void BackendTimeProxy::setMaintainTime(const int64_t &source)
{
    maintainTime = static_cast<time_t>(source);
    timerMaintain->start();
}
	
CURLcode BackendTimeProxy::getBETimeFromLink(time_t &beTime, bool isFallback, time_t preBe)
{
    // To avoid multiple return
    CURLcode res   = CURLE_OK;
    CURLcode check = CURLE_OK;
	
    do {
        if (stopGettingBeTime) { // If received shutdown event, stop getting BE time
            nf::info(ActiveLogContext, "Stop getting BE time from BE server because of shutdown event");
            break;
        }

        // Notify that getting BE time status is changed
        nf::Executor::mainThread()->post([this] {
            if (manager) {
                manager->onGettingBeTimeStatusChanged(true);
            }
        });
		
        nf::info(ActiveLogContext, "Start getting BE time from BE server");
        std::string readBuffer;
		
        // NOLINTBEGIN
        auto curl = curl_easy_init();
        if(!curl) {
            break;
        }
        return 1;
	        
        int32_t checkRes = 0;
        static std::string headerUserAgent = getHeaderUserAgent();
		
        auto headers = static_cast<struct curl_slist *>(nullptr);
        //headers
        headers = curl_slist_append(headers, headerHost.c_str());
        headers = curl_slist_append(headers, headerUserAgent.c_str());
        //Header add
        if (headers == nullptr) {
            curl_easy_cleanup(curl);
            break;
        }
        checkRes |= curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		
        // Enable SSL peer verification to ensure the authenticity of the SSL certificate
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, CURLPARAM_SSL_VERIFYPEER);
		
        // Verify that the SSL certificate's common name or subject alternative name matches the hostname
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, CURLPARAM_SSL_VERIFYHOST);
		
        // Set a callback function to customize the SSL/TLS context
        curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, curl_callback::CurlCallback::ssl_ctx_callback);
		
        nf::info(ActiveLogContext, "SSL context data: isFallback = {}, preBe = {}, OCSP = {}", nf::pub(isFallback), nf::pub(preBe), nf::pub(isOcspProvisioningStatus));
        bool isEnabledVerifyOcsp = !isFallback && (preBe == 0) && isOcspProvisioningStatus;
        if (isEnabledVerifyOcsp) {
            // Enable auto verify OCSP response
            nf::info(ActiveLogContext, "Enable auto verify OCSP response");
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, CURLPARAM_SSL_VERIFYSTATUS);
        }
		
        // Set the SSL context data
        curl_callback::SSLConfig config;
        config.isFallBack = isFallback;
        config.beTime = preBe;
        config.isOcspVerifyStatus = isOcspProvisioningStatus;
        nf::info(ActiveLogContext, "SSL context data: isFallback = {}, beTime = {}", nf::pub(config.isFallBack), nf::pub(config.beTime));
        curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, &config);
		
        // process get certificate from CURLPARAM_SSLCERT and CURLPARAM_SSLCHAINCERT
        std::vector<unsigned char> combinedCertData;
        processGetCertificate({CURLPARAM_SSLCERT.data(), CURLPARAM_SSLCHAINCERT.data()}, combinedCertData);
        nf::info(ActiveLogContext, "SSL certificate size = {}", nf::pub(combinedCertData.size()));
		
        curl_blob sslCertBlob{};
        sslCertBlob.data = combinedCertData.data();
        sslCertBlob.len = combinedCertData.size();
        sslCertBlob.flags = CURL_BLOB_NOCOPY;
		
        checkRes |= curl_easy_setopt(curl, CURLOPT_SSLCERT_BLOB, &sslCertBlob);
		
        checkRes |= curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, nf::pub(SSL_KEY_TYPE.data()));
        checkRes |= curl_easy_setopt(curl, CURLOPT_SSLENGINE, nf::pub(ENGINE_ID.data()));
		
        static std::string sslKey = getSslKey(ENGINE_ID.data(), TOKEN_LABEL.data(), PIN_PATH.data(), KEY_ID.data());
        checkRes |= curl_easy_setopt(curl, CURLOPT_SSLKEY, nf::pub(sslKey.c_str()));
        checkRes |= curl_easy_setopt(curl, CURLOPT_CAINFO, nf::pub(CURLPARAM_CAINFO.data()));
        // Print CA path
        string caPath = resolveSymlink(CURLPARAM_CAINFO.data());
        nf::info(ActiveLogContext, "CA path = {}", nf::pub(caPath));
        //cipher
        checkRes |= curl_easy_setopt(curl, CURLOPT_SSL_CIPHER_LIST, nf::pub(CURLPARAM_SSL_CIPHER_LIST.data()));
        //TLS v1.3
        checkRes |= curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_3);
        //show detail connection information option
        nf::info(ActiveLogContext, "used urls {}; source = {}", nf::pub(secureTimeURL.c_str()), isUrlDefault ? "default" : "provisioning");
        checkRes |= curl_easy_setopt(curl, CURLOPT_URL, nf::pub(secureTimeURL.c_str()));
        //send all data to this function
        checkRes |= curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteCallback);
        checkRes |= curl_easy_setopt(curl, CURLOPT_HEADERDATA, &readBuffer);
		
        /* SEC_CON_381
            * The connection timeout shall be at least 7 seconds.
            * The request timeout shall be at least 25 seconds.
         */
        // 7L for connection timeout
        checkRes |= curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CURLPARAM_TIMEOUT_CONNECTION);
		
        // 25L for request timeout
        checkRes |= curl_easy_setopt(curl, CURLOPT_TIMEOUT, CURLPARAM_TIMEOUT_REQUEST);
		
        // Set the debug function
        checkRes |= curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_callback);
        // Enable verbose output
        checkRes |= curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		
        if(checkRes != CURLE_OK) { // NOLINT(cppcoreguidelines-pro-type-vararg)
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            break;
        }
		
        nf::info(ActiveLogContext, "Start curl_easy_perform");
        nf::info(ActiveLogContext, "send Http Request {}", nf::pub(secureTimeURL.c_str()));
        res = curl_easy_perform(curl);
        if(res != check) {
            nf::info(ActiveLogContext, "got Http Response {}; return error code = {} - {}, x-httpstime = N/A", nf::pub(secureTimeURL.c_str()), nf::pub(res), nf::pub(curl_easy_strerror(res)));
            checkFailReason(res);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            break;
        }
		
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        // NOLINTEND
		
        if (!readBuffer.empty()) {
            nf::info(ActiveLogContext, "Getting BE Time Success!!!");
            beTime = getTimeFromBuffer(readBuffer);
            nf::info(ActiveLogContext, "got Http Response {}; return code = {} - {}, {}, converted BETime = {}", nf::pub(secureTimeURL.c_str()), nf::pub(res), nf::pub(curl_easy_strerror(res)), nf::pub(readBuffer), nf::pub(beTime));
        } else {
            nf::info(ActiveLogContext, "Get BackendTime Failed");
            nf::info(ActiveLogContext, "got Http Response {}; return error code = {} - {}, x-httpstime = N/A", nf::pub(secureTimeURL.c_str()), nf::pub(res), nf::pub(curl_easy_strerror(res)));
        }
    } while(false); // To avoid multiple return
	
    nf::info(ActiveLogContext, "End getting BE time from BE server");
    // Notify that getting BE time status is changed
    nf::Executor::mainThread()->post([this] {
        if (manager) {
            manager->onGettingBeTimeStatusChanged(false);
        }
    });
	
    return res;
}
	
time_t BackendTimeProxy::getTimeFromBuffer(const std::string &readBuffer) {
    time_t beTime = 0;
	
    if (!readBuffer.empty()) {
        // Convert time from readBuffer to epoch time
        std::string sec = readBuffer.substr(SECOND_INDEX,SECOND_LENGTH);
        std::string strMilli = readBuffer.substr(MIL_SECOND_INDEX);
        auto stopstring = static_cast<char*>(nullptr);
        // Convert from decimal string to number, then 10 is the base of conversation
        int64_t beSec   = strtoll(sec.c_str(), &stopstring, BASE_CONVERSION);
        int64_t beMilli = strtoll(strMilli.c_str(), &stopstring, BASE_CONVERSION);
	
        if(isValidDate(beSec, beMilli)) {
                // 1000 and 1000000 is for second and milisecond converting
                beTime = static_cast<time_t> ((beSec * 1000 + beMilli / 1000000) - START_TIME_MIL_SECOND); // After 1970-01-01 00:00:00
                beTime = beTime/1000; // Convert to second time
        } else {
            failReason = "Data corrupted in BEproxy";
        }
    }
	
    return beTime;
}
	
bool BackendTimeProxy::isValidDate(const int64_t beSec, const int64_t beMilli) {
    return (beSec != 0 && beSec != LLONG_MIN && beSec != LLONG_MAX &&
            beMilli != 0 && beMilli != LLONG_MIN && beMilli != LLONG_MAX);
}
	
std::string BackendTimeProxy::getHeaderUserAgent() {
    nf::info(ActiveLogContext, "Start get header user agent");
	
    std::string ecu = "ICON";
    std::string puMeasure;
    std::string vehicleVariant;
    std::string countryVariant;
    std::string softwareVersion;
    std::stringstream headerUserAgent;
	
    countryVariant = UtilTMGR::getCountryVariant();
	
    softwareVersion = loadVersion(VERSION_PATH.data());
	
    headerUserAgent << HEADER_USER_AGENT << ecu << ";" << puMeasure << ";" << vehicleVariant << ";" << countryVariant << ";" << softwareVersion;
    nf::info(ActiveLogContext, "Header user agent: {}", nf::pub(headerUserAgent.str().c_str()));
	
    return headerUserAgent.str();
}
	
std::string BackendTimeProxy::getSslKey(const std::string& engineId,
                                        const std::string& tokenLabel,
                                        const std::string& pinPath,
                                        const std::string& keyId) {
    auto pin = loadPin(pinPath);
	
    std::stringstream pkcs11Uri;
    pkcs11Uri << engineId << ":token=" << tokenLabel << ";id=" << keyId
              << ";pin-value=";
	
    // Convert pin value to HEX
    for (char character : pin) {
        pkcs11Uri << "%" << std::hex << std::setfill('0') << std::setw(2)
               << static_cast<int>(character);
    }
	
    return pkcs11Uri.str();
}
	
std::string BackendTimeProxy::loadPin(const std::filesystem::path& path) {
    nf::info(ActiveLogContext, "Start loading PIN");
    return loadFile(path);
}
	
std::string BackendTimeProxy::loadVersion(const std::filesystem::path& path) {
    nf::info(ActiveLogContext, "Start loading Version");
	
    std::string line;
    std::string version;
    std::string fileContent = loadFile(path);
    std::stringstream fileStream(fileContent);
	
    while (getline(fileStream, line)) {
        if (line.find("VERSION") == 0) {
            version = line.substr(line.find('=') + 2); // skip '=' and the opening quote
            version.pop_back(); // Remove the closing quote
            break;
        }
    }
	
    if (version.empty()) {
        nf::error(ActiveLogContext, "File: {} invalid format.", nf::pub(path.c_str()));
    } else {
        nf::info(ActiveLogContext, "Open {} file successfully", nf::pub(path));
    }
	    
    return version;
}
	
std::string BackendTimeProxy::loadFile(const std::filesystem::path& path) {
    nf::info(ActiveLogContext, "Start loading file {}", nf::pub(path.c_str()));
    std::string content;
    try {
        do {
            if (!fileSystemWrapper->exists(path)) {
                nf::error(ActiveLogContext, "File: {} doesn't exist.", nf::pub(path.c_str()));
                break;
            }
	
            if (!fileSystemWrapper->isRegularFile(path)) {
                nf::error(ActiveLogContext, "Path: {} does not point to a regular file.", nf::pub(path.c_str()));
                break;
            }
	
            std::ifstream file(path);
            if (!file) {
                nf::error(ActiveLogContext, "File: {} cannot be opened.", nf::pub(path.c_str()));
                break;
            }
	
            std::stringstream stringStream;
            stringStream << file.rdbuf();
            content = std::move(stringStream).str();
	
            nf::info(ActiveLogContext, "File: {} loaded successfully, size: {}", nf::pub(path.c_str()), nf::pub(content.size()));
        } while (false);
    }
    catch (const std::exception& e) {
        nf::error(ActiveLogContext, "Failed to read file: {} with an exception: {}.", nf::pub(path.c_str()), nf::pub(e.what()));
    }
	
    return content;
}
	
void BackendTimeProxy::checkFailReason(CURLcode res) {
    if (res == CURLE_OPERATION_TIMEDOUT ||
        res == CURLE_COULDNT_RESOLVE_HOST ||
        res == CURLE_COULDNT_CONNECT) {
        failReason = "No data connection";
    } else if (res == CURLE_SSL_CERTPROBLEM) {
        failReason = "Authentication to BE fails. ( certs )";
    } else {
        failReason = curl_easy_strerror(res);
    }
}
	
void BackendTimeProxy::processRequestBeTime(bool isDataConnectionChanged, bool isPicking) {
    do {
        if (!isDataConnected) {
            failReason = "No data connection";
            // Processing with BE time is 0
            processingData(0, isDataConnectionChanged, isPicking);
            break;
        }
	
        if (isPicking) {
            nf::info(ActiveLogContext, "Just isPicking");
            break;
        }
	
        if (stopGettingBeTime) {
            nf::info(ActiveLogContext, "Stop getting BE time because of received shutdown event");
            break;
        }
	
        if (failedConnections >= MAX_RETRY_FALLBACK_FAILED) {
            if (cachedRetryTimes == MAX_DELAY_TIME_FALLBACK_FAILED && !isDataConnectionChanged) {
                failedConnections = 0;
                cachedRetryTimes = 0;
            } else {
                nf::info(ActiveLogContext, "Max retries. Try again in the next lifecycle (after cold boot or wake up from suspend mode) or at least after 24 hours.");
                break;
            }
        }
	
        // Invoked in a context of a worker thread
        mThread.post([this, isDataConnectionChanged] {
            bool isNeedFallback = false;
            this->handleBETime(isDataConnectionChanged, isNeedFallback);
            if (isNeedFallback) {
                nf::Executor::thisThread()->post([this, isDataConnectionChanged] {
                    nf::info(ActiveLogContext, "Fallback custom TLS handshake");
                    bool isFailedFallback = false;
                    this->handleFallbackBETime(isDataConnectionChanged, isFailedFallback);
                    if (isFailedFallback) {
                        nf::info(ActiveLogContext, "handleFallbackBETime is failed, number of failed connections is {} and next time to request to BE is after {} seconds", nf::pub(failedConnections), nf::pub(cachedRetryTimes));
                    }
                    nf::info(ActiveLogContext, "End Fallback custom TLS handshake");
                });
            }
        });
    } while(false);
}
	
void BackendTimeProxy::handleBETime(bool isDataConnectionChanged, bool &isNeedFallback) {
    CURLcode result = CURLE_OK;
    time_t resBeTime = 0;
	
    nf::info(ActiveLogContext, "Start {}", nf::pub(__func__));
	
    result = this->getBETimeFromLink(resBeTime);
    if (result == CURLE_OK) {
        if (resBeTime == 0) {
            nf::Executor::mainThread()->post([this, resBeTime, isDataConnectionChanged] {
                this->processingData(resBeTime, isDataConnectionChanged);
            });
            isNeedFallback = false;
        } else {
            result = this->getBETimeFromLink(resBeTime, false, resBeTime);
            if (result == CURLE_OK) {
                nf::Executor::mainThread()->post([this, resBeTime, isDataConnectionChanged] {
                    this->processingData(resBeTime, isDataConnectionChanged);
                });
                isNeedFallback = false;
            } else {
                isNeedFallback = true;
            }
        }
    } else {
        isNeedFallback = true;
    }
	
    if (!isNeedFallback) {
        failedConnections = 0;
    }
	
    nf::info(ActiveLogContext, "End {}", nf::pub(__func__));
}
	
void BackendTimeProxy::handleFallbackBETime(bool isDataConnectionChanged, bool &isFailedFallback) {
    CURLcode result = CURLE_OK;
    time_t resBeTime = 0;
	
    nf::info(ActiveLogContext, "Start {}", nf::pub(__func__));
	
    result = this->getBETimeFromLink(resBeTime, true);
    if (result == CURLE_OK && resBeTime != 0) {
        result = this->getBETimeFromLink(resBeTime, false, resBeTime);
        if (result == CURLE_OK && resBeTime != 0) {
            nf::Executor::mainThread()->post([this, resBeTime, isDataConnectionChanged] {
                this->processingData(resBeTime, isDataConnectionChanged);
            });
            isFailedFallback = false;
        } else {
            isFailedFallback = true;
        }
    } else {
        isFailedFallback = true;
    }
	
    if (isFailedFallback) {
        failedConnections++;
        
        cachedRetryTimes = 0;
        if (failedConnections < MAX_RETRY_FALLBACK_FAILED) {
            cachedRetryTimes = BackendTimeProxy::calculateExpotentialBackoff(failedConnections);
        } else {
            cachedRetryTimes = MAX_DELAY_TIME_FALLBACK_FAILED;
        }
	
        nf::Executor::mainThread()->post([this, isDataConnectionChanged] {
            this->processingData(0, isDataConnectionChanged);
        });
    } else {
        failedConnections = 0;
    }
	
    nf::info(ActiveLogContext, "End {}", nf::pub(__func__));
}
	
ProxyTime BackendTimeProxy::selectDataSource(bool isDataConnectionChanged, bool isPicking)
{
    ProxyTime be_time(Proxy::BE);
	
    auto shmName = "/BET";
    int shm_fd = shm_open(shmName, O_RDONLY, 0644);
    if (shm_fd == -1) {
        nf::error(ActiveLogContext, "shm_open error");
        checkingRealData = true;
        nf::info(ActiveLogContext, "Enabled real data testing getting from Backend");
        processRequestBeTime(isDataConnectionChanged, isPicking);
    } else {
        auto ptr = static_cast<char*>(mmap(nullptr, SIZE, PROT_READ, MAP_SHARED, shm_fd, static_cast<off_t>(0)));
        // MAP_FAILED comes from system
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast, performance-no-int-to-ptr)
        if (ptr == static_cast<char*>(MAP_FAILED)) {
            nf::error(ActiveLogContext, "Map failed");
            checkingRealData = true;
            nf::info(ActiveLogContext, "Enabled real data testing getting from Backend");
            processRequestBeTime(isDataConnectionChanged, isPicking);
        } else {
            std::string str(ptr);
            if(str.length() < strlen("enableRealData")) {
                nf::info(ActiveLogContext, "Invalid sldd command for BETime");
                failReason = "Data corrupted in BEproxy";
            } else if (strncmp(str.c_str(), "enableRealData", strlen("enableRealData")) == 0) {
                checkingRealData = true;
                nf::info(ActiveLogContext, "Enabled real data testing getting from Backend");
                processRequestBeTime(isDataConnectionChanged, isPicking);
            } else {
                nf::info(ActiveLogContext, "Disabled real data testing, getting from sldd");
                checkingRealData = false;
                /* Get data from ptr, casted to char value */
                std::string str(ptr);
                munmap(ptr, SIZE);
                nf::info(ActiveLogContext, "Sldd buffer: {}", nf::pub(str));
                be_time = fromString(str);
                if (be_time.date.year == 0u) {
                    failReason = "Data corrupted in BEproxy";
                }
                nf::info(ActiveLogContext, "{} - {} - {}", nf::pub(__FUNCTION__), nf::pub(__LINE__), nf::pub(str));
            }
        }
    }
	
    return be_time;
}
	
bool BackendTimeProxy::proccessData(const time_t beTime, bool isDataConnectionChanged) {
    nf::info(ActiveLogContext, "Start processing received data.");
    if(beTime != 0) {
        setMaintainTime(beTime);
        if (delay != BackendTimeProxy::DelayRequestSuccess) {
            delay = BackendTimeProxy::DelayRequestSuccess;
            timer->stop();
            timer->setInterval(delay);
            timer->start();
        }
	
        timeInfo = convertToProxyTime(maintainTime);
        // Print log for be time received.
        auto beTimeInfo = convertToProxyTime(beTime);
        nf::info(ActiveLogContext, "BE time received: UTC-Date={}-{}-{}|UTC-Time={}:{}:{}:{}us"
                        ,nf::pub(+beTimeInfo.date.year), nf::pub(+beTimeInfo.date.month), nf::pub(+beTimeInfo.date.day), nf::pub(+beTimeInfo.time.hour), nf::pub(+beTimeInfo.time.minute), nf::pub(+beTimeInfo.time.second), nf::pub(+beTimeInfo.time.nanosecond));
        nf::info(ActiveLogContext, "BE time info: UTC-Date={}-{}-{}|UTC-Time={}:{}:{}:{}us"
                        ,nf::pub(+timeInfo.date.year), nf::pub(+timeInfo.date.month), nf::pub(+timeInfo.date.day), nf::pub(+timeInfo.time.hour), nf::pub(+timeInfo.time.minute), nf::pub(+timeInfo.time.second), nf::pub(+timeInfo.time.nanosecond));
        manager->forward(timeInfo);
    } else {
        // Update cycle and start timer to request BE time again if BE time changes from availability to unvailability
        bool isNeedUpdateTimerInterval = false;
        if (failedConnections > 0 && isDataConnected) {
            delay = std::chrono::seconds(cachedRetryTimes);
            isNeedUpdateTimerInterval = true;
        } else {
            if (delay != BackendTimeProxy::DelayRequestFailed) {
                delay = BackendTimeProxy::DelayRequestFailed;
                isNeedUpdateTimerInterval = true;
            }
        }
	        
        if (isNeedUpdateTimerInterval) {
            timer->stop();
            timer->setInterval(delay);
            timer->start();
        }
	
        timerMaintain->stop();
        maintainTime = 0;
        if (isDataConnectionChanged) {
            nf::info(ActiveLogContext, "Proxy=BETime|FlagValue=0|UTC-Date=N/A|UTC-Time=N/A|TTAQ=N/A|TTSQ=N/A|TimeZone=N/A|rQT=N/A|TRIGGERreason=DataConnectionStateChanged|FAILreason={}", nf::pub(failReason));
        }
    }
    nf::info(ActiveLogContext, "End processing received data.");
    return true;
}
	
void BackendTimeProxy::check(bool isDataConnectionChanged, bool isPicking)
{
    // handle getting data from BE Link
    timeInfo = selectDataSource(isDataConnectionChanged, isPicking);
	
    // if used sldd data, print log for fake data
    if (checkingRealData == false && isDataConnectionChanged == false) {
        checkSlddData();
    }
}
	
bool BackendTimeProxy::processingData(const time_t beTime, bool isDataConnectionChanged, bool isSkipCheckingData)
{
    bool result = false;
	
    // Handle processing data
    result = proccessData(beTime, isDataConnectionChanged);
	
    if (!isSkipCheckingData) {
        // print log for real data
        checkRealData();
    }
	
    return result;
}
	
void BackendTimeProxy::checkSlddData() 
{
    if (timeInfo.date.year == 0u) {
        delay = BackendTimeProxy::DelayRequestFailed;
        nf::info(ActiveLogContext, "Proxy=BETime|FlagValue=0|UTC-Date=N/A|UTC-Time=N/A|TTAQ=N/A|TTSQ=N/A|TimeZone=N/A|rQT=N/A|TRIGGERreason=Cyclic{}sec|FAILreason={}", nf::pub(duration_cast<seconds>(delay).count()), nf::pub(failReason));
        nf::info(ActiveLogContext, "BETime is not available, changed the delay value: {}", nf::pub(duration_cast<seconds>(delay).count()));
    } else {
        delay = BackendTimeProxy::DelayRequestSuccess;
        nf::info(ActiveLogContext, "Proxy=BETime|FlagValue=1|UTC-Date={}-{}-{}|UTC-Time={}:{}:{}:{}us|TTAQ=N/A|TTSQ=VeryTrustworthy|TimeZone={}|rQT=N/A|TRIGGERreason=Cyclic{}sec"
                ,nf::pub(+timeInfo.date.year), nf::pub(+timeInfo.date.month), nf::pub(+timeInfo.date.day), nf::pub(+timeInfo.time.hour), nf::pub(+timeInfo.time.minute), nf::pub(+timeInfo.time.second), nf::pub(+timeInfo.time.nanosecond), nf::pub(+timeInfo.tz), nf::pub(duration_cast<seconds>(delay).count()));
        manager->forward(timeInfo);
    }
    timer->stop();
    timer->setInterval(delay);
    timer->start();
}
	
void BackendTimeProxy::checkRealData() 
{
    if(maintainTime == 0u) {
        delay = (failedConnections > 0 && isDataConnected) ? std::chrono::seconds(cachedRetryTimes) : BackendTimeProxy::DelayRequestFailed;
        nf::info(ActiveLogContext, "Proxy=BETime|FlagValue=0|UTC-Date=N/A|UTC-Time=N/A|TTAQ=N/A|TTSQ=N/A|TimeZone=N/A|rQT=N/A|TRIGGERreason=Cyclic{}sec|FAILreason={}", nf::pub(duration_cast<seconds>(delay).count()), nf::pub(failReason));
        nf::info(ActiveLogContext, "BETime is not available, changed the delay value: {}", nf::pub(duration_cast<seconds>(delay).count()));
    } else {
        timeInfo = convertToProxyTime(maintainTime);
        delay = BackendTimeProxy::DelayRequestSuccess;
        nf::info(ActiveLogContext, "Proxy=BETime|FlagValue=1|UTC-Date={}-{}-{}|UTC-Time={}:{}:{}:{}us|TTAQ=N/A|TTSQ=VeryTrustworthy|TimeZone={}|rQT=N/A|TRIGGERreason=Cyclic{}sec"
                ,nf::pub(+timeInfo.date.year), nf::pub(+timeInfo.date.month), nf::pub(+timeInfo.date.day), nf::pub(+timeInfo.time.hour), nf::pub(+timeInfo.time.minute), nf::pub(+timeInfo.time.second), nf::pub(+timeInfo.time.nanosecond), nf::pub(+timeInfo.tz), nf::pub(duration_cast<seconds>(delay).count()));
        manager->forward(timeInfo);
    }
    timer->stop();
    timer->setInterval(delay);
    timer->start();
}
	
ProxyTime BackendTimeProxy::convertToProxyTime(time_t timedata)
{
    ProxyTime beTime(Proxy::BE);
    struct tm ti = {};
    auto ret = gmtime_r(&timedata, &ti);
    if(ret) {
        beTime.date.year    = static_cast<uint16_t>(ti.tm_year + 1900);
        beTime.date.month   = static_cast<uint8_t>(ti.tm_mon + 1);
        beTime.date.day     = static_cast<uint8_t>(ti.tm_mday);
        beTime.time.hour    = static_cast<uint8_t>(ti.tm_hour);
        beTime.time.minute  = static_cast<uint8_t>(ti.tm_min);
        beTime.time.second  = static_cast<uint8_t>(ti.tm_sec);
    }
	
    return beTime;
}
	
/*
    * [0,…,2^c-1]+constant
 */
uint32_t BackendTimeProxy::calculateExpotentialBackoff(uint32_t failedNumber)
{
    std::atomic<uint32_t> result{0};
	
    uint32_t maxBackoff = (1u << failedNumber) - 1; // 2^c - 1
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, maxBackoff);
    auto rdBackoff = static_cast<uint32_t>(dis(gen));
	
    nf::info(ActiveLogContext, "{}: Random of Expotential Backoff {}", __func__, nf::pub(rdBackoff));
	
    /*
        * SEC_CON_500
        * random in range [0, 2^c-1] (minutes) + constant (30 seconds)
        * 60 is for converting to seconds
    */
    result = (rdBackoff * 60) + EXPONENTIAL_BACKOFF_CONSTANT;
	
    return result.load();
}
	
void BackendTimeProxy::tryGet()
{
    Executor::mainThread()->post([this]{ check(); });
}
	
optional<ProxyTime> BackendTimeProxy::get(bool isPicking)
{
    bool retCheck = false;
	
    timeInfo = selectDataSource(false, true);
    if(checkingRealData == false) {
        if (timeInfo.date.year == 0u) {
            if(isPicking) {
                nf::info(ActiveLogContext, "Proxy=BETime|FlagValue=0|UTC-Date=N/A|UTC-Time=N/A|TTAQ=N/A|TTSQ=N/A|TimeZone=N/A|rQT=N/A|TRIGGERreason=RequestGetUpdate|FAILreason={}", nf::pub(failReason));
                if (delay != BackendTimeProxy::DelayRequestFailed) {
                    delay = BackendTimeProxy::DelayRequestFailed;
                    timer->stop();
                    timer->setInterval(delay);
                    timer->start();
                }
            } else {
                nf::info(ActiveLogContext, "getBETime: UTC-Date=N/A|UTC-Time=N/A|TTAQ=N/A|TTSQ=N/A|TimeZone=N/A");
            }
        } else {
            retCheck = true;
            if (isPicking) {
                nf::info(ActiveLogContext, "Proxy=BETime|FlagValue=1|UTC-Date={}-{}-{}|UTC-Time={}:{}:{}:{}us|TTAQ=N/A|TTSQ=VeryTrustworthy|TimeZone={}|rQT=N/A|TRIGGERreason=RequestGetUpdate"
                            ,nf::pub(+timeInfo.date.year), nf::pub(+timeInfo.date.month), nf::pub(+timeInfo.date.day), nf::pub(+timeInfo.time.hour), nf::pub(+timeInfo.time.minute), nf::pub(+timeInfo.time.second), nf::pub(+timeInfo.time.nanosecond), nf::pub(+timeInfo.tz));
            } else {
                nf::info(ActiveLogContext, "getBETime: UTC-Date={}-{}-{}|UTC-Time={}:{}:{}:{}us|TimeZone={}"
                            ,nf::pub(+timeInfo.date.year), nf::pub(+timeInfo.date.month), nf::pub(+timeInfo.date.day), nf::pub(+timeInfo.time.hour), nf::pub(+timeInfo.time.minute), nf::pub(+timeInfo.time.second), nf::pub(+timeInfo.time.nanosecond), nf::pub(+timeInfo.tz));
            }
        }
    } else {
        if (maintainTime == 0u) {
            if (isPicking) {
                nf::info(ActiveLogContext, "Proxy=BETime|FlagValue=0|UTC-Date=N/A|UTC-Time=N/A|TTAQ=N/A|TTSQ=N/A|TimeZone=N/A|rQT=N/A|TRIGGERreason=RequestGetUpdate|FAILreason={}", nf::pub(failReason));
            } else {
                nf::info(ActiveLogContext, "getBETime: UTC-Date=N/A|UTC-Time=N/A|TTAQ=N/A|TTSQ=N/A|TimeZone=N/A");
            }
        } else {
            retCheck = true;
            timeInfo = convertToProxyTime(maintainTime);
            if (isPicking) {
                nf::info(ActiveLogContext, "Proxy=BETime|FlagValue=1|UTC-Date={}-{}-{}|UTC-Time={}:{}:{}:{}us|TTAQ=N/A|TTSQ=VeryTrustworthy|TimeZone={}|rQT=N/A|TRIGGERreason=RequestGetUpdate"
                            ,nf::pub(+timeInfo.date.year), nf::pub(+timeInfo.date.month), nf::pub(+timeInfo.date.day), nf::pub(+timeInfo.time.hour), nf::pub(+timeInfo.time.minute), nf::pub(+timeInfo.time.second), nf::pub(+timeInfo.time.nanosecond), nf::pub(+timeInfo.tz));
            } else {
                nf::info(ActiveLogContext, "getBETime: UTC-Date={}-{}-{}|UTC-Time={}:{}:{}:{}us|TimeZone={}"
                            ,nf::pub(+timeInfo.date.year), nf::pub(+timeInfo.date.month), nf::pub(+timeInfo.date.day), nf::pub(+timeInfo.time.hour), nf::pub(+timeInfo.time.minute), nf::pub(+timeInfo.time.second), nf::pub(+timeInfo.time.nanosecond), nf::pub(+timeInfo.tz));
            }
        }
    }
	
    return retCheck ? std::make_optional(timeInfo) : std::nullopt;
}
	
void BackendTimeProxy::setSecureTimeURL(std::string &stsurl)
{
    nf::info(ActiveLogContext, "Set secure time URL.");
    secureTimeURL = stsurl;
    isUrlDefault = false;
    setHeaderHost(stsurl);
    nf::info(ActiveLogContext, "Secure Time URL received: {}", nf::pub(secureTimeURL.c_str()));
}
	
void BackendTimeProxy::setHeaderHost(const std::string &stsurl) {
    nf::info(ActiveLogContext, "Set header host.");
	
    // If can't find "://", URL is invalid.
    size_t protocolEnd = stsurl.find("://");
    if (protocolEnd == std::string::npos) {
        nf::info(ActiveLogContext, "URL is invalid.");
    } else {
        // Ignore "://", need to add 3
        size_t hostStart = protocolEnd + 3;
	
        // Find position of '/' after host
        size_t hostEnd = stsurl.find('/', hostStart);
        if (hostEnd == std::string::npos) {
            // If not found '/', host is the remaining part of stsurl
            hostEnd = stsurl.length();
        }
	
        // Get host from stsurl
        std::string host = stsurl.substr(hostStart, hostEnd - hostStart);
	
        // Remove port from host
        size_t portPos = host.find(':');
        if (portPos != std::string::npos) {
            // headerHost should be from index 0 of host to before of ':'
            host = host.substr(0, portPos);
        }
	
        headerHost = "Host: " + host;
    }
	    
    nf::info(ActiveLogContext, "Header host: {}", nf::pub(headerHost.c_str()));
}
	
void BackendTimeProxy::onNetworkInfoChanged(bool isDataConnectedNotified) {
    if (isDataConnectedNotified == true && isDataConnected == false) {
        isDataConnected = true;
        // Wait 4 seconds to resolving host name is successful. 
        // Also after ECU is connected to network, BETime must be requested getting time in no more than 5 seconds
        nf::Executor::mainThread()->post(4s, [this] {
            check(true, false);
        });
        nf::info(ActiveLogContext, "Data Connection State: CONNECTED");
    }
    if (isDataConnectedNotified == false && isDataConnected == true) {
        isDataConnected = false;
        nf::Executor::mainThread()->post([this] {
            check(true, false);
        });
        nf::info(ActiveLogContext, "Data Connection State: NOT CONNECTED");
    }
}
	
void BackendTimeProxy::destroy() {
    if (mConsumerMobileConnection) {
        mConsumerMobileConnection->destroy();
    }
	
    if (mSP25ConsumerMobileConnection) {
        mSP25ConsumerMobileConnection->destroy();
    }
}
	
void BackendTimeProxy::processWakeUpEvent() {
    // After Wakeup maintain time will be set to 0
    nf::info(ActiveLogContext, "Force clear BE maintained time after wakeup");
    timerMaintain->stop();
    maintainTime = 0;
    nf::info(ActiveLogContext, "Proxy=BETime|FlagValue=0|UTC-Date=N/A|UTC-Time=N/A|TTAQ=N/A|TTSQ=N/A|TimeZone=N/A|rQT=N/A|TRIGGERreason=Wakeup|FAILreason=InitForWakingUp");
	
    /*
        * SEC_CON_382 If the connection is not established after the 7th request the ECU shall try again in the next lifecycle 
        * (after wake up from suspend mode)
    */
    failedConnections = 0;
	
    // Register mobile connection provider
    registerSomeIPProvider();
	    
    // Force request BETime again to get new update source status
    nf::info(ActiveLogContext, "Request get real BE time again after wakeup");
    if (timer->isRunning() == true) {
        timer->stop();
    }
	
    // Request to get BE time when waking up (Ensure getting WUC time first)
    Executor::mainThread()->post(500ms, [this] () { tryGet(); });
}
	
void BackendTimeProxy::receivedShutDownEventChanged(const TimeMgrType::ShutdownType& type) {
    nf::info(ActiveLogContext, "Received shutdown event: {}", nf::pub(static_cast<uint8_t>(type)));
    if (type == TimeMgrType::ShutdownType::Postponable) {
        // Stop requesting BE time
        stopGettingBeTime = true;
        unregisterSomeIPProvider();
    } else {
        // Start requesting BE time
        stopGettingBeTime = false;
    }
}
	
void BackendTimeProxy::receivedProvisioningDataChanged(ProvisioningData &provisioningData) {
    nf::info(ActiveLogContext, "Start {}", nf::pub(__func__));
    if (provisioningData.secureTimeProvisioningDataSection.has_value()) {
        auto url = provisioningData.secureTimeProvisioningDataSection.value().stsUrl;
        setSecureTimeURL(url);
	
        isOcspProvisioningStatus = provisioningData.secureTimeProvisioningDataSection.value().isOcsp;
    }
    nf::info(ActiveLogContext, "End {}", nf::pub(__func__));
}
	
void BackendTimeProxy::processGetCertificate(const std::vector<std::filesystem::path>& paths, std::vector<unsigned char>& combinedCertData) {
    nf::info(ActiveLogContext, "Start {}", nf::pub(__func__));
    for (const auto& path : paths) {
        std::string certData = loadFile(path);
        if (certData.empty()) {
            nf::info(ActiveLogContext, "Certificate file is error: {}", nf::pub(path.c_str()));
            break;
        }
	
        combinedCertData.insert(combinedCertData.end(), certData.begin(), certData.end());
    }
    nf::info(ActiveLogContext, "End {}", nf::pub(__func__));
}
	
string BackendTimeProxy::resolveSymlink(const string& symlinkPath) {
    nf::info(ActiveLogContext, "Read symlink path: {}", nf::pub(symlinkPath.c_str()));
    string result;
    try {
        result = filesystem::read_symlink(symlinkPath).string();
    } catch (const filesystem::filesystem_error& e) {
        result = "Error: " + string(e.what());
    }
    nf::info(ActiveLogContext, "Resolved symlink path: {}", nf::pub(result.c_str()));
    return result;
}
	
void BackendTimeProxy::registerSomeIPProvider() {
    if(mSPVerRVP != SP_VER::SP_25) {
        if (!mConsumerMobileConnection) {
            nf::info(ActiveLogContext, "[BackendTimeProxy] Create SP21 MobileConnection proxy...");
            mConsumerMobileConnection = IConsummerMobileConnection::create(manager);
        }
    } else {
        if (!mSP25ConsumerMobileConnection) {
            nf::info(ActiveLogContext, "[BackendTimeProxy] Create SP25 MobileConnection proxy...");
            mSP25ConsumerMobileConnection = ISP25ConsummerMobileConnection::create(manager);
        }
    }
}
	
void BackendTimeProxy::unregisterSomeIPProvider() {
    if (mConsumerMobileConnection) {
        mConsumerMobileConnection.reset();
    }
	
    if (mSP25ConsumerMobileConnection) {
        mSP25ConsumerMobileConnection.reset();
    }
}