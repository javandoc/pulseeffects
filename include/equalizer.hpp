#ifndef EQUALIZER_HPP
#define EQUALIZER_HPP

#include <gio/gio.h>
#include <gst/gst.h>
#include <gst/insertbin/gstinsertbin.h>
#include <sigc++/sigc++.h>
#include <array>
#include <iostream>
#include <thread>

class Equalizer {
   public:
    Equalizer(std::string tag, std::string schema);
    ~Equalizer();

    bool is_enabled = false;

    std::string log_tag, name = "equalizer";

    GstElement *plugin, *bin, *equalizer;

    void init_equalizer();

   private:
    bool is_installed;

    GSettings* settings;

    std::vector<std::thread> threads;

    std::vector<GObject*> bands;

    void bind_to_gsettings();
};

#endif
