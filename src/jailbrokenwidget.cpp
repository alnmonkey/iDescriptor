#include "jailbrokenwidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#undef slots
#undef signals
#include <QtConcurrent/QtConcurrentRun> // Correct header for QtConcurrent::run
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#include <windows.h>
#else
#define EXPORT __attribute__((visibility("default")))
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "frida-core.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTextStream>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libssh/callbacks.h>
#include <libssh/libssh.h>
#include <libssh/sftp.h> // keep for other code, but not used for transfer
#include <string>
#include <zip.h>

namespace fs = std::filesystem;

// Forward declarations
struct Application {
    std::string name;
    std::string identifier;
    unsigned int pid;
};

struct SSHConfig {
    std::string host = "localhost";
    int port = 2222;
    std::string user = "root";
    std::string password = "alpine";
    std::string key_filename = "";
};

// Progress callback types
using GeneralProgressCallback =
    std::function<void(const std::string &message, const std::string &type,
                       int current, int total)>;
using DownloadProgressCallback = std::function<void(int progress)>;
using ZipProgressCallback = std::function<void(int progress)>;
using ErrorCallback = std::function<void(const std::string &message)>;

class IOSDumper
{
private:
    // Callback storage
    std::vector<GeneralProgressCallback> general_progress_callbacks;
    std::vector<DownloadProgressCallback> download_progress_callbacks;
    std::vector<ZipProgressCallback> zip_progress_callbacks;
    std::vector<ErrorCallback> error_callbacks;

    // Threading
    std::mutex callback_mutex;
    std::condition_variable finished_cv;
    std::mutex finished_mutex;
    std::atomic<bool> finished{false};

    // Device and session management
    FridaDevice *device = nullptr;
    FridaSession *session = nullptr;
    ssh_session sshSession = nullptr;
    ssh_scp scp = nullptr; // Renamed from sftp

    // File management
    std::string temp_dir;
    std::string payload_path = "./Payload";
    std::map<std::string, std::string> file_dict;

    // JavaScript dump script content
    std::string dump_js_content;

    void notify_progress(const std::string &message,
                         const std::string &type = "info", int current = 0,
                         int total = 100);
    void notify_download_progress(int current, int total);
    void notify_zip_progress(int current, int total);
    void notify_error(const std::string &message);

    bool init_frida();
    bool get_usb_device();
    bool connect_ssh(const SSHConfig &config);
    bool create_directory(const std::string &path);
    std::vector<Application> get_applications();
    bool open_target_app(const std::string &name_or_bundleid, bool kill_running,
                         std::string &display_name,
                         std::string &bundle_identifier);
    bool download_file(const std::string &remote_path,
                       const std::string &local_path, bool recursive = false);
    bool generate_ipa(const std::string &display_name);
    bool load_dump_script();

    static void on_frida_message(FridaScript *script, const char *message,
                                 GBytes *data, gpointer user_data);
    void handle_frida_message(const std::string &message);

    void cleanup();

public:
    IOSDumper();
    ~IOSDumper();

    // Callback management
    void add_general_progress_callback(GeneralProgressCallback callback);
    void add_download_progress_callback(DownloadProgressCallback callback);
    void add_zip_progress_callback(ZipProgressCallback callback);
    void add_error_callback(ErrorCallback callback);
    void clear_progress_callbacks();

    // Legacy callback support
    void add_progress_callback(GeneralProgressCallback callback);

    // Main functionality
    bool dump_app(const std::string &name_or_bundleid,
                  const std::string &output_name = "",
                  const SSHConfig &ssh_config = SSHConfig(),
                  bool kill_running = true);
    std::vector<std::pair<std::string, std::string>> get_all_applications();

    // Legacy function
    bool start(const std::string &name_or_bundleid, bool kill_running = true,
               const SSHConfig &ssh_config = SSHConfig());
};

// Implementation
IOSDumper::IOSDumper()
{
    // Initialize temp directory
    temp_dir = fs::temp_directory_path();
    payload_path = temp_dir + "/Payload";

    // Initialize Frida
    init_frida();

    // Load dump script (this would be embedded or loaded from file)
    load_dump_script();
}

IOSDumper::~IOSDumper() { cleanup(); }

bool IOSDumper::init_frida()
{
    frida_init();
    return true;
}

void IOSDumper::cleanup()
{
    if (session) {
        frida_session_detach_sync(session, nullptr, nullptr);
        g_object_unref(session);
        session = nullptr;
    }

    if (device) {
        g_object_unref(device);
        device = nullptr;
    }

    if (scp) {
        ssh_scp_free(scp);
        scp = nullptr;
    }
    if (sshSession) {
        ssh_disconnect(sshSession);
        ssh_free(sshSession);
        sshSession = nullptr;
    }

    // Clean up temp directory
    if (fs::exists(payload_path)) {
        fs::remove_all(payload_path);
    }

    frida_deinit();
}

void IOSDumper::add_general_progress_callback(GeneralProgressCallback callback)
{
    std::lock_guard<std::mutex> lock(callback_mutex);
    general_progress_callbacks.push_back(callback);
}

void IOSDumper::add_download_progress_callback(
    DownloadProgressCallback callback)
{
    std::lock_guard<std::mutex> lock(callback_mutex);
    download_progress_callbacks.push_back(callback);
}

void IOSDumper::add_zip_progress_callback(ZipProgressCallback callback)
{
    std::lock_guard<std::mutex> lock(callback_mutex);
    zip_progress_callbacks.push_back(callback);
}

void IOSDumper::add_error_callback(ErrorCallback callback)
{
    std::lock_guard<std::mutex> lock(callback_mutex);
    error_callbacks.push_back(callback);
}

void IOSDumper::clear_progress_callbacks()
{
    std::lock_guard<std::mutex> lock(callback_mutex);
    general_progress_callbacks.clear();
    download_progress_callbacks.clear();
    zip_progress_callbacks.clear();
    error_callbacks.clear();
}

void IOSDumper::add_progress_callback(GeneralProgressCallback callback)
{
    add_general_progress_callback(callback);
}

void IOSDumper::notify_progress(const std::string &message,
                                const std::string &type, int current, int total)
{
    std::lock_guard<std::mutex> lock(callback_mutex);
    for (const auto &callback : general_progress_callbacks) {
        try {
            callback(message, type, current, total);
        } catch (...) {
            // Ignore callback errors
        }
    }
}

void IOSDumper::notify_download_progress(int current, int total)
{
    int progress = (total > 0) ? (current * 100 / total) : 0;
    std::lock_guard<std::mutex> lock(callback_mutex);
    for (const auto &callback : download_progress_callbacks) {
        try {
            callback(progress);
        } catch (...) {
            // Ignore callback errors
        }
    }
}

void IOSDumper::notify_zip_progress(int current, int total)
{
    int progress = (total > 0) ? (current * 100 / total) : 0;
    std::lock_guard<std::mutex> lock(callback_mutex);
    for (const auto &callback : zip_progress_callbacks) {
        try {
            callback(progress);
        } catch (...) {
            // Ignore callback errors
        }
    }
}

void IOSDumper::notify_error(const std::string &message)
{
    std::lock_guard<std::mutex> lock(callback_mutex);
    for (const auto &callback : error_callbacks) {
        try {
            callback(message);
        } catch (...) {
            // Ignore callback errors
        }
    }
}

bool IOSDumper::get_usb_device()
{
    FridaDeviceManager *device_manager = frida_device_manager_new();

    while (!device) {
        notify_progress("Waiting for USB device...", "waiting");

        GError *error = nullptr;
        FridaDeviceList *devices = frida_device_manager_enumerate_devices_sync(
            device_manager, nullptr, &error);

        if (error) {
            notify_error("Failed to enumerate devices: " +
                         std::string(error->message));
            g_error_free(error);
            g_object_unref(device_manager);
            return false;
        }

        int device_count = frida_device_list_size(devices);
        printf("Found %d devices\n", device_count);
        for (int i = 0; i < device_count; i++) {
            FridaDevice *dev = frida_device_list_get(devices, i);
            // FridaDeviceType type = frida_device_get_type(dev);
            GType dev_type = frida_device_get_dtype(dev);
            printf("Type of device %d: %s\n", i, g_type_name(dev_type));
            if (dev_type == FRIDA_DEVICE_TYPE_USB) {
                // Handle USB device
                device = dev;
                g_object_ref(device);
                notify_progress("Connected to device: " +
                                    std::string(frida_device_get_name(device)),
                                "success");
                break;
            }
            g_object_unref(dev);
        }

        g_object_unref(devices);

        if (!device) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    g_object_unref(device_manager);
    return true;
}

bool IOSDumper::connect_ssh(const SSHConfig &config)
{
    notify_progress("Connecting to device via SSH...", "info");

    sshSession = ssh_new();
    if (!sshSession) {
        notify_error("Failed to create SSH session");
        return false;
    }
    try {
        ssh_options_set(sshSession, SSH_OPTIONS_HOST, config.host.c_str());
        ssh_options_set(sshSession, SSH_OPTIONS_PORT, &config.port);
        ssh_options_set(sshSession, SSH_OPTIONS_USER, config.user.c_str());
        int rc = ssh_connect(sshSession);
        if (rc != SSH_OK) {
            notify_error("Failed to connect to SSH: " +
                         std::string(ssh_get_error(sshSession)));
            return false;
        }

        // Accept new host keys automatically
        rc = ssh_session_is_known_server(sshSession);
        printf("SSH session is known server: %d\n", rc);
        if (rc == SSH_SERVER_NOT_KNOWN || rc == SSH_SERVER_FILE_NOT_FOUND) {
            // Accept the key and update known_hosts
            notify_progress("Trying to add host key to known_hosts", "info");
            rc = ssh_session_update_known_hosts(sshSession);
            if (rc != SSH_OK) {
                notify_error("Failed to accept new SSH host key");
                ssh_free(sshSession);
                return false;
            }
        } else if (rc == SSH_SERVER_KNOWN_OK) {
            // Host key is already known, continue
        } else if (rc == SSH_SERVER_KNOWN_CHANGED) {
            notify_error("SSH host key has changed!");
            return false;
        } else if (rc == SSH_SERVER_FOUND_OTHER) {
            notify_error("SSH host key type mismatch!");
            return false;
        } else if (rc == SSH_SERVER_ERROR) {
            notify_error("SSH server error: " +
                         std::string(ssh_get_error(sshSession)));
            return false;
        }

        if (!config.key_filename.empty()) {
            ssh_key key;
            rc = ssh_pki_import_privkey_file(config.key_filename.c_str(),
                                             nullptr, nullptr, nullptr, &key);
            if (rc != SSH_OK) {
                notify_error("Failed to load SSH key");
                return false;
            }
            rc = ssh_userauth_publickey(sshSession, nullptr, key);
            ssh_key_free(key);
        } else {
            rc = ssh_userauth_password(sshSession, nullptr,
                                       config.password.c_str());
        }

        if (rc != SSH_AUTH_SUCCESS) {
            notify_error("SSH authentication failed");
            return false;
        }

        // Use SCP instead of SFTP
        scp = ssh_scp_new(sshSession, SSH_SCP_READ, "/");
        if (!scp) {
            notify_error("Failed to create SCP session");
            return false;
        }

        rc = ssh_scp_init(scp);
        if (rc != SSH_OK) {
            notify_error("Failed to initialize SCP");
            ssh_scp_free(scp);
            scp = nullptr;
            return false;
        }

        return true;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        notify_error("Exception during SSH connection: " +
                     std::string(e.what()));
        ssh_free(sshSession);
        return false;
    }
}

bool IOSDumper::create_directory(const std::string &path)
{
    if (fs::exists(path)) {
        fs::remove_all(path);
    }

    try {
        fs::create_directories(path);
        return true;
    } catch (const std::exception &e) {
        notify_error("Error creating directory: " + std::string(e.what()));
        return false;
    }
}

std::vector<Application> IOSDumper::get_applications()
{
    std::vector<Application> apps;

    if (!device) {
        notify_error("No device connected");
        return apps;
    }

    GError *error = nullptr;
    FridaApplicationList *applications =
        frida_device_enumerate_applications_sync(device, nullptr, nullptr,
                                                 &error);

    if (error) {
        notify_error("Failed to enumerate applications: " +
                     std::string(error->message));
        g_error_free(error);
        return apps;
    }

    int app_count = frida_application_list_size(applications);
    notify_progress("Found " + std::to_string(app_count) + " applications",
                    "info");

    for (int i = 0; i < app_count; i++) {
        FridaApplication *app = frida_application_list_get(applications, i);
        Application application;
        application.name = frida_application_get_name(app);
        application.identifier = frida_application_get_identifier(app);
        application.pid = frida_application_get_pid(app);
        apps.push_back(application);
        g_object_unref(app);
    }

    g_object_unref(applications);
    return apps;
}

bool IOSDumper::open_target_app(const std::string &name_or_bundleid,
                                bool kill_running, std::string &display_name,
                                std::string &bundle_identifier)
{
    notify_progress("Starting target app: " + name_or_bundleid, "info");

    auto applications = get_applications();
    unsigned int pid = 0;

    for (const auto &app : applications) {
        if (app.identifier == name_or_bundleid ||
            app.name == name_or_bundleid) {
            pid = app.pid;
            display_name = app.name;
            bundle_identifier = app.identifier;
            break;
        }
    }

    if (bundle_identifier.empty()) {
        notify_error("App not found: " + name_or_bundleid);
        return false;
    }

    GError *error = nullptr;

    if (pid == 0) {
        notify_progress("Spawning app: " + display_name, "info");
        pid = frida_device_spawn_sync(device, bundle_identifier.c_str(),
                                      nullptr, nullptr, &error);
        if (error) {
            notify_error("Failed to spawn app: " + std::string(error->message));
            g_error_free(error);
            return false;
        }

        // session = frida_device_attach_sync(device, pid, nullptr, &error);
        session =
            frida_device_attach_sync(device, pid, nullptr, nullptr, &error);
        if (error) {
            notify_error("Failed to attach to app: " +
                         std::string(error->message));
            g_error_free(error);
            return false;
        }

        frida_device_resume_sync(device, pid, nullptr, &error);
        if (error) {
            notify_error("Failed to resume app: " +
                         std::string(error->message));
            g_error_free(error);
            return false;
        }
    } else {
        if (kill_running) {
            notify_progress("Killing running app: " + display_name +
                                " (PID: " + std::to_string(pid) + ")",
                            "info");
            frida_device_kill_sync(device, pid, nullptr, &error);
            if (error) {
                notify_error("Failed to kill app: " +
                             std::string(error->message));
                g_error_free(error);
                return false;
            }

            notify_progress("Spawning app: " + display_name, "info");
            pid = frida_device_spawn_sync(device, bundle_identifier.c_str(),
                                          nullptr, nullptr, &error);
            if (error) {
                notify_error("Failed to spawn app: " +
                             std::string(error->message));
                g_error_free(error);
                return false;
            }

            session =
                frida_device_attach_sync(device, pid, nullptr, nullptr, &error);
            if (error) {
                notify_error("Failed to attach to app: " +
                             std::string(error->message));
                g_error_free(error);
                return false;
            }

            frida_device_resume_sync(device, pid, nullptr, &error);
            if (error) {
                notify_error("Failed to resume app: " +
                             std::string(error->message));
                g_error_free(error);
                return false;
            }
        } else {
            notify_progress("Attaching to running app: " + display_name +
                                " (PID: " + std::to_string(pid) + ")",
                            "info");
            session =
                frida_device_attach_sync(device, pid, nullptr, nullptr, &error);
            if (error) {
                notify_error("Failed to attach to app: " +
                             std::string(error->message));
                g_error_free(error);
                return false;
            }
        }
    }

    return true;
}

bool IOSDumper::load_dump_script()
{
    QFile file(":resources/dump.js"); // Use resource path
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        notify_error("Failed to open dump.js from resources");
        return false;
    }
    QTextStream in(&file);
    dump_js_content = in.readAll().toStdString();
    file.close();
    return true;
}
void IOSDumper::on_frida_message(FridaScript *script, const char *message,
                                 GBytes *data, gpointer user_data)
{
    printf("Frida message received: %s\n", message);
    IOSDumper *dumper = static_cast<IOSDumper *>(user_data);
    dumper->handle_frida_message(std::string(message));
}

void IOSDumper::handle_frida_message(const std::string &message)
{
    printf("Received message: %s\n", message.c_str());

    // Use Qt JSON parsing
    QByteArray jsonBytes(message.c_str(), message.size());
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        notify_error(
            "Failed to parse JSON message: " +
            std::string(parseError.errorString().toUtf8().constData()));
        return;
    }

    QJsonObject obj = doc.object();

    if (obj.contains("payload")) {
        QJsonObject payload = obj.value("payload").toObject();

        if (payload.contains("dump") && payload.contains("path")) {
            QString dump_path = payload.value("dump").toString();
            QString origin_path = payload.value("path").toString();

            std::string local_path =
                payload_path + "/" +
                fs::path(dump_path.toStdString()).filename().string();
            download_file(dump_path.toStdString(), local_path);

            int index = origin_path.indexOf(".app/");
            if (index != -1) {
                std::string dict_key =
                    fs::path(dump_path.toStdString()).filename().string();
                std::string dict_value =
                    origin_path.mid(index + 5).toStdString();
                file_dict[dict_key] = dict_value;
            }
        }
        if (payload.contains("app")) {
            QString app_path = payload.value("app").toString();
            std::string local_path =
                payload_path + "/" +
                fs::path(app_path.toStdString()).filename().string();
            download_file(app_path.toStdString(), local_path, true);

            file_dict["app"] =
                fs::path(app_path.toStdString()).filename().string();
        }
        if (payload.contains("done")) {
            notify_progress("Download completed", "success");
            finished = true;
            finished_cv.notify_all();
        }
    } else if (obj.contains("done")) {
        notify_progress("Download completed", "success");
        finished = true;
        finished_cv.notify_all();
    }
}

bool IOSDumper::download_file(const std::string &remote_path,
                              const std::string &local_path, bool recursive)
{
    // SCP only supports file transfer, not recursive directory download.
    // For recursive, you would need to implement directory traversal via SSH
    // commands. Here we only support single file download.

    int rc = ssh_scp_pull_request(scp);
    if (rc != SSH_SCP_REQUEST_NEWFILE) {
        notify_error("Failed to request file via SCP: " + remote_path);
        return false;
    }

    size_t file_size = ssh_scp_request_get_size(scp);

    std::ofstream out(local_path, std::ios::binary);
    if (!out) {
        notify_error("Failed to create local file: " + local_path);
        ssh_scp_deny_request(scp, "Local file error");
        return false;
    }

    char buffer[131072];
    size_t total = 0;
    while (total < file_size) {
        size_t to_read = std::min(sizeof(buffer), file_size - total);
        int nread = ssh_scp_read(scp, buffer, to_read);
        if (nread < 0) {
            notify_error("Error reading from SCP");
            out.close();
            return false;
        }
        out.write(buffer, nread);
        total += nread;
        notify_download_progress(total, file_size);
    }

    out.close();
    ssh_scp_accept_request(scp);

    notify_progress("Downloaded " + remote_path + " to " + local_path,
                    "success");

    return true;
}

bool IOSDumper::generate_ipa(const std::string &display_name)
{
    std::string ipa_filename = display_name + ".ipa";
    notify_progress("Generating \"" + ipa_filename + "\"", "info");

    try {
        // Move files according to file_dict
        std::string app_name = file_dict["app"];
        fs::path app_path = fs::path(payload_path) / app_name;

        // If app_path exists and is not a directory, remove it
        if (fs::exists(app_path) && !fs::is_directory(app_path)) {
            fs::remove(app_path);
            fs::create_directory(app_path);
        } else if (!fs::exists(app_path)) {
            fs::create_directory(app_path);
        }

        for (const auto &[key, value] : file_dict) {
            if (key != "app") {
                fs::path from_path = fs::path(payload_path) / key;
                fs::path to_path = app_path / value;
                fs::create_directories(to_path.parent_path());
                fs::rename(from_path, to_path);
            }
        }

        // Use libzip to create IPA file
        std::string ipa_path = (fs::current_path() / ipa_filename).string();
        int errorp;
        zip_t *zip = zip_open(ipa_path.c_str(), ZIP_CREATE | ZIP_EXCL, &errorp);
        if (!zip) {
            notify_error("Failed to create IPA file with libzip");
            return false;
        }

        // Add all files in Payload recursively
        for (const auto &entry :
             fs::recursive_directory_iterator(payload_path)) {
            if (entry.is_regular_file()) {
                std::string file_path = entry.path().string();
                // Archive name should be relative to temp_dir/Payload
                std::string archive_name =
                    fs::relative(entry.path(), fs::path(payload_path)).string();

                zip_source_t *source =
                    zip_source_file(zip, file_path.c_str(), 0, -1);
                if (!source) {
                    notify_error("Failed to create zip source for: " +
                                 file_path);
                    zip_close(zip);
                    return false;
                }

                if (zip_file_add(zip, archive_name.c_str(), source,
                                 ZIP_FL_OVERWRITE) < 0) {
                    notify_error("Failed to add file to zip: " + file_path);
                    zip_source_free(source);
                    zip_close(zip);
                    return false;
                }
            }
        }

        zip_close(zip);

        // Remove Payload directory
        fs::remove_all(payload_path);

        notify_progress("Successfully created " + ipa_filename, "success");
        return true;
    } catch (const std::exception &e) {
        notify_error("Error generating IPA: " + std::string(e.what()));
        return false;
    }
}

bool IOSDumper::dump_app(const std::string &name_or_bundleid,
                         const std::string &output_name,
                         const SSHConfig &ssh_config, bool kill_running)
{
    finished = false;

    try {
        // Get device
        if (!get_usb_device()) {
            return false;
        }

        // Connect SSH
        if (!connect_ssh(ssh_config)) {
            return false;
        }

        // Create payload directory
        if (!create_directory(payload_path)) {
            return false;
        }

        // Open target app
        std::string display_name, bundle_identifier;
        if (!open_target_app(name_or_bundleid, kill_running, display_name,
                             bundle_identifier)) {
            return false;
        }

        // Create and load dump script
        GError *error = nullptr;
        FridaScript *script = frida_session_create_script_sync(
            session, dump_js_content.c_str(), nullptr, nullptr, &error);
        if (error) {
            notify_error("Failed to create script: " +
                         std::string(error->message));
            g_error_free(error);
            return false;
        }

        // Set up message handler before loading
        g_signal_connect(script, "message", G_CALLBACK(on_frida_message), this);

        // Load the script before posting
        frida_script_load_sync(script, nullptr, &error);
        if (error) {
            notify_error("Failed to load script: " +
                         std::string(error->message));
            g_error_free(error);
            g_object_unref(script);
            return false;
        }

        // Now post the message to start the dump
        frida_script_post(script, "{\"type\": \"dump\"}", nullptr);

        notify_progress("Starting dump of " + display_name, "info");
        // frida_script_load_sync(script, "{\"type\":\"dump\"}", nullptr,
        // &error); REMOVE THIS LINE: frida_script_load_sync(script, nullptr,
        // &error);
        if (error) {
            notify_error("Failed to start dump: " +
                         std::string(error->message));
            g_error_free(error);
            g_object_unref(script);
            return false;
        }

        // Wait for completion
        std::unique_lock<std::mutex> lock(finished_mutex);
        finished_cv.wait(lock, [this] { return finished.load(); });

        // Generate IPA

        // IOSDumper::download_file

        std::string output_ipa =
            output_name.empty() ? display_name : output_name;
        if (output_ipa.ends_with(".ipa")) {
            output_ipa = output_ipa.substr(0, output_ipa.length() - 4);
        }

        bool success = generate_ipa(output_ipa);

        g_object_unref(script);
        return success;
    } catch (const std::exception &e) {
        notify_error("Error: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::pair<std::string, std::string>>
IOSDumper::get_all_applications()
{
    std::vector<std::pair<std::string, std::string>> result;

    if (!get_usb_device()) {
        return result;
    }

    auto applications = get_applications();
    for (const auto &app : applications) {
        result.emplace_back(app.name, app.identifier);
    }

    return result;
}

bool IOSDumper::start(const std::string &name_or_bundleid, bool kill_running,
                      const SSHConfig &ssh_config)
{
    return dump_app(name_or_bundleid, "", ssh_config, kill_running);
}

// C API for shared library
extern "C" {
EXPORT IOSDumper *create_dumper() { return new IOSDumper(); }

EXPORT void destroy_dumper(IOSDumper *dumper) { delete dumper; }

EXPORT bool dump_app_c(IOSDumper *dumper, const char *name_or_bundleid,
                       const char *output_name, const char *ssh_host,
                       int ssh_port, const char *ssh_user,
                       const char *ssh_password, bool kill_running)
{
    SSHConfig config;
    config.host = ssh_host ? ssh_host : "localhost";
    config.port = ssh_port > 0 ? ssh_port : 2222;
    config.user = ssh_user ? ssh_user : "root";
    config.password = ssh_password ? ssh_password : "alpine";

    return dumper->dump_app(name_or_bundleid, output_name ? output_name : "",
                            config, kill_running);
}

EXPORT void add_progress_callback_c(IOSDumper *dumper,
                                    void (*callback)(const char *message,
                                                     const char *type,
                                                     int current, int total))
{
    dumper->add_general_progress_callback([callback](const std::string &message,
                                                     const std::string &type,
                                                     int current, int total) {
        callback(message.c_str(), type.c_str(), current, total);
    });
}

EXPORT void add_error_callback_c(IOSDumper *dumper,
                                 void (*callback)(const char *message))
{
    dumper->add_error_callback(
        [callback](const std::string &message) { callback(message.c_str()); });
}
}

JailbrokenWidget::JailbrokenWidget(QWidget *parent) : QWidget{parent}
{
    // Initialization code can go here
    setWindowTitle("Jailbroken Device Widget");
    setMinimumSize(400, 300);
    setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc; "
                  "border-radius: 5px;");

    setLayout(new QVBoxLayout(this));
    QPushButton *infoButton = new QPushButton("Show Jailbreak Info", this);
    connect(infoButton, &QPushButton::clicked, this, [this]() {
        // Run dump logic in a background thread to avoid UI freeze
        QtConcurrent::run([] {
            IOSDumper dumper;
            dumper.add_progress_callback([](const std::string &message,
                                            const std::string &type,
                                            int current, int total) {
                printf("[%s] %s (%d/%d)\n", type.c_str(), message.c_str(),
                       current, total);
            });
            dumper.add_error_callback([](const std::string &message) {
                fprintf(stderr, "Error: %s\n", message.c_str());
            });
            SSHConfig ssh_config;
            ssh_config.host = "0.0.0.0";
            ssh_config.port = 3333;
            ssh_config.user = "root";
            ssh_config.password = "alpine";
            dumper.dump_app("Firefox", "FirefoxDump", ssh_config, true);
        });
    });
}
