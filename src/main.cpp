// g++ main.cpp `pkg-config gtkmm-3.0 --cflags --libs` -o rec

// g++ main.cpp `pkg-config gtkmm-3.0 --cflags --libs` -o rec
//records video and mic :)) 

#include <gtkmm.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>

class MainWindow : public Gtk::Window {
public:
  MainWindow() {
    set_default_size(400, 200);
    set_title("Screen Recorder");

    button_record.set_label("Record");
    button_record.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_record_button_clicked));

    button_stop.set_label("Stop");
    button_stop.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_stop_button_clicked));
    button_stop.set_sensitive(false);

    label_fps.set_text("FPS:");
    label_resolution.set_text("Resolution:");

    entry_fps.set_text("30");
    entry_resolution.set_text("1920x1080");

    check_audio.set_label("Record Audio");
    check_audio.set_active(false); // Set the initial state of the check box

    grid.attach(label_fps, 0, 0, 1, 1);
    grid.attach(label_resolution, 0, 1, 1, 1);
    grid.attach(entry_fps, 1, 0, 1, 1);
    grid.attach(entry_resolution, 1, 1, 1, 1);
    grid.attach(check_audio, 0, 2, 2, 1);
    grid.attach(button_record, 0, 3, 1, 1);
    grid.attach(button_stop, 1, 3, 1, 1);

    add(grid);
    show_all();
  }

protected:
  void on_record_button_clicked() {
    std::string fps_str = entry_fps.get_text();
    int fps = std::stoi(fps_str);

    std::string resolution_str = entry_resolution.get_text();
    std::size_t pos = resolution_str.find('x');
    int width = std::stoi(resolution_str.substr(0, pos));
    int height = std::stoi(resolution_str.substr(pos + 1));

    Gtk::FileChooserDialog dialog("Save Output File", Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.set_transient_for(*this);
    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Save", Gtk::RESPONSE_OK);

    // Set default filename and location
    dialog.set_current_name("recording.mp4");
    dialog.set_current_folder(Glib::get_home_dir());

    int result = dialog.run();
    if (result == Gtk::RESPONSE_OK) {
      std::string output_file = dialog.get_filename();

      button_record.set_sensitive(false);
      button_stop.set_sensitive(true);

      pid_t pid = fork();
      if (pid == 0) {
        std::string command = "ffmpeg -f x11grab -s " + std::to_string(width) + "x" + std::to_string(height) +
                              " -framerate " + std::to_string(fps) + " -i :0.0+0,0";

        // Add audio recording option if the check box is checked
        if (check_audio.get_active()) {
          command += " -f pulse -ac 2 -i default";
        }

        command += " -c:v libx264 -preset ultrafast -c:a aac " + output_file;

        int status = std::system(command.c_str());
        if (status == 0) {
          std::cout << "Recording finished. Output file: " << output_file << std::endl;
        } else {
          std::cout << "Recording failed." << std::endl;
        }
        exit(0);
      }
    }
  }

  void on_stop_button_clicked() {
    std::string command = "pkill ffmpeg";
    std::system(command.c_str());

    button_record.set_sensitive(true);
    button_stop.set_sensitive(false);
  }

  Gtk::Grid grid;
  Gtk::Button button_record;
  Gtk::Button button_stop;
  Gtk::Label label_fps, label_resolution;
  Gtk::Entry entry_fps, entry_resolution;
  Gtk::CheckButton check_audio; // Check box for audio recording
};

int main(int argc, char* argv[]) {
  auto app = Gtk::Application::create(argc, argv, "org.example.screencapture");
  MainWindow window;
  return app->run(window);
}