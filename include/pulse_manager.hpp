#ifndef PULSE_MANAGER_HPP
#define PULSE_MANAGER_HPP

#include <glibmm.h>
#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>
#include <sigc++/sigc++.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <memory>

struct myServerInfo {
  std::string server_name;
  std::string server_version;
  std::string default_sink_name;
  std::string default_source_name;
  std::string protocol;
  std::string format;
  std::string channel_map;
  uint rate;
  uint8_t channels;
};

struct mySinkInfo {
  std::string name;
  uint index;
  std::string description;
  uint owner_module;
  uint monitor_source;
  std::string monitor_source_name;
  uint rate;
  std::string format;
  std::string active_port;
};

struct mySourceInfo {
  std::string name;
  uint index;
  std::string description;
  uint rate;
  std::string format;
  std::string active_port;
};

struct myModuleInfo {
  std::string name;
  uint index;
  std::string argument;
};

struct myClientInfo {
  std::string name;
  uint index;
  std::string binary;
};

struct AppInfo {
  std::string app_type;
  uint index;
  std::string name;
  std::string icon_name;
  std::string media_name;
  uint8_t channels;
  double volume;
  uint rate;
  std::string resampler;
  std::string format;
  int mute;
  bool connected;
  bool visible;
  uint buffer;
  uint latency;
  int corked;
  bool wants_to_play;
};

class ParseAppInfo;

class PulseManager {
 public:
  PulseManager();
  PulseManager(const PulseManager&) = delete;
  auto operator=(const PulseManager&) -> PulseManager& = delete;
  PulseManager(const PulseManager&&) = delete;
  auto operator=(const PulseManager &&) -> PulseManager& = delete;
  ~PulseManager();

  pa_threaded_mainloop* main_loop = nullptr;

  myServerInfo server_info;
  std::shared_ptr<mySinkInfo> apps_sink_info;
  std::shared_ptr<mySinkInfo> mic_sink_info;

  auto get_sink_info(const std::string& name) -> std::shared_ptr<mySinkInfo>;
  auto get_source_info(const std::string& name) -> std::shared_ptr<mySourceInfo>;

  std::vector<std::string> blocklist_in;   // for input effects
  std::vector<std::string> blocklist_out;  // for output effects

  void find_sink_inputs();
  void find_source_outputs();
  void find_sinks();
  void find_sources();
  auto move_sink_input_to_pulseeffects(const std::string& name, uint idx) -> bool;
  auto remove_sink_input_from_pulseeffects(const std::string& name, uint idx) -> bool;
  auto move_source_output_to_pulseeffects(const std::string& name, uint idx) -> bool;
  auto remove_source_output_from_pulseeffects(const std::string& name, uint idx) -> bool;
  void set_sink_input_volume(const std::string& name, uint idx, uint8_t channels, uint value);
  void set_sink_input_mute(const std::string& name, uint idx, bool state);
  void set_source_output_volume(const std::string& name, uint idx, uint8_t channels, uint value);
  void set_source_output_mute(const std::string& name, uint idx, bool state);
  void get_sink_input_info(uint idx);
  void update_server_info(const pa_server_info* info);
  void get_modules_info();
  void get_clients_info();

  sigc::signal<void, std::shared_ptr<mySourceInfo>> source_added;
  sigc::signal<void, std::shared_ptr<mySourceInfo>> source_changed;
  sigc::signal<void, uint> source_removed;
  sigc::signal<void, std::shared_ptr<mySinkInfo>> sink_added;
  sigc::signal<void, std::shared_ptr<mySinkInfo>> sink_changed;
  sigc::signal<void, uint> sink_removed;
  sigc::signal<void, std::string> new_default_sink;
  sigc::signal<void, std::string> new_default_source;
  sigc::signal<void, std::shared_ptr<AppInfo>> sink_input_added;
  sigc::signal<void, std::shared_ptr<AppInfo>> sink_input_changed;
  sigc::signal<void, uint> sink_input_removed;
  sigc::signal<void, std::shared_ptr<AppInfo>> source_output_added;
  sigc::signal<void, std::shared_ptr<AppInfo>> source_output_changed;
  sigc::signal<void, uint> source_output_removed;
  sigc::signal<void> server_changed;
  sigc::signal<void, std::shared_ptr<myModuleInfo>> module_info;
  sigc::signal<void, std::shared_ptr<myClientInfo>> client_info;

 private:
  std::string log_tag = "pulse_manager: ";

  bool context_ready = false;

  pa_mainloop_api* main_loop_api = nullptr;
  pa_context* context = nullptr;

  std::array<std::string, 7> blocklist_apps = {
      "PulseEffectsWebrtcProbe", "gsd-media-keys", "GNOME Shell", "libcanberra", "Screenshot", "speech-dispatcher"};

  std::array<std::string, 4> blocklist_media_name = {"pulsesink probe", "bell-window-system", "audio-volume-change",
                                                     "screen-capture"};

  std::array<std::string, 1> blocklist_media_role = {"event"};

  std::array<std::string, 4> blocklist_app_id = {"com.github.wwmm.pulseeffects.sinkinputs",
                                                 "com.github.wwmm.pulseeffects.sourceoutputs",
                                                 "org.PulseAudio.pavucontrol", "org.gnome.VolumeControl"};

  static void context_state_cb(pa_context* ctx, void* data);

  void subscribe_to_events();

  void get_server_info();

  auto get_default_sink_info() -> std::shared_ptr<mySinkInfo>;

  auto get_default_source_info() -> std::shared_ptr<mySourceInfo>;

  auto load_sink(const std::string& name, const std::string& description, uint rate) -> std::shared_ptr<mySinkInfo>;

  void load_apps_sink();

  void load_mic_sink();

  auto load_module(const std::string& name, const std::string& argument) -> bool;

  void unload_module(uint idx);

  void unload_sinks();

  void drain_context();

  void new_app(const pa_sink_input_info* info);

  void new_app(const pa_source_output_info* info);

  void changed_app(const pa_sink_input_info* info);

  void changed_app(const pa_source_output_info* info);

  static void print_app_info(const std::shared_ptr<AppInfo>& info);

  auto app_is_connected(const pa_sink_input_info* info) -> bool;

  auto app_is_connected(const pa_source_output_info* info) -> bool;

  static auto get_latency(const pa_sink_input_info* info) -> uint { return info->sink_usec; }

  static auto get_latency(const pa_source_output_info* info) -> uint { return info->source_usec; }

  template <typename T>
  auto parse_app_info(const T& info) -> std::shared_ptr<AppInfo> {
    std::string app_name;
    std::string media_name;
    std::string media_role;
    std::string app_id;
    auto ai = std::make_shared<AppInfo>();
    bool forbidden_app = false;

    auto prop = pa_proplist_gets(info->proplist, "application.name");

    if (prop != nullptr) {
      app_name = prop;

      forbidden_app =
          std::find(std::begin(blocklist_apps), std::end(blocklist_apps), app_name) != std::end(blocklist_apps);

      if (forbidden_app) {
        return nullptr;
      }
    }

    prop = pa_proplist_gets(info->proplist, "media.name");

    if (prop != nullptr) {
      media_name = prop;

      if (app_name.empty()) {
        app_name = media_name;
      }

      forbidden_app = std::find(std::begin(blocklist_media_name), std::end(blocklist_media_name), media_name) !=
                      std::end(blocklist_media_name);

      if (forbidden_app) {
        return nullptr;
      }
    }

    prop = pa_proplist_gets(info->proplist, "media.role");

    if (prop != nullptr) {
      media_role = prop;

      forbidden_app = std::find(std::begin(blocklist_media_role), std::end(blocklist_media_role), media_role) !=
                      std::end(blocklist_media_role);

      if (forbidden_app) {
        return nullptr;
      }
    }

    prop = pa_proplist_gets(info->proplist, "application.id");

    if (prop != nullptr) {
      app_id = prop;

      forbidden_app =
          std::find(std::begin(blocklist_app_id), std::end(blocklist_app_id), app_id) != std::end(blocklist_app_id);

      if (forbidden_app) {
        return nullptr;
      }
    }

    prop = pa_proplist_gets(info->proplist, "application.icon_name");

    std::string icon_name;

    if (prop != nullptr) {
      icon_name = prop;
    } else {
      prop = pa_proplist_gets(info->proplist, "media.icon_name");

      if (prop != nullptr) {
        if (std::strcmp(prop, "audio-card-bluetooth") ==
            0) {  // there is no GTK icon with this name given by Pulseaudio =/
        } else {
          icon_name = "bluetooth-symbolic";
        }
      } else {
        icon_name = "audio-x-generic-symbolic";
      }
    }

    // Connection flag: it specifies only the primary state that can be enabled/disabled by the user
    ai->connected = app_is_connected(info);

    // Initialize visibility to true, it will be properly updated forward
    ai->visible = true;

    // linear volume
    ai->volume = 100.0 * (static_cast<double>(pa_cvolume_max(&info->volume)) / PA_VOLUME_NORM);

    if (info->resample_method) {
      ai->resampler = info->resample_method;
    } else {
      ai->resampler = "none";
    }

    ai->format = pa_sample_format_to_string(info->sample_spec.format);

    ai->index = info->index;
    ai->name = app_name;
    ai->media_name = media_name;
    ai->icon_name = icon_name;
    ai->channels = info->volume.channels;
    ai->rate = info->sample_spec.rate;
    ai->mute = info->mute;
    ai->buffer = info->buffer_usec;
    ai->latency = get_latency(info);
    ai->corked = info->corked;
    ai->wants_to_play = ai->connected && !ai->corked;

    return ai;
  }
};

#endif
