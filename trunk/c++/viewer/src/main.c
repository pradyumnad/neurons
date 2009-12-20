/*
 * Initial main.c file generated by Glade. Edit as required.
 * Glade will not overwrite this file.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#ifdef WITH_GLEW
#include <GL/glew.h>
#endif

#include <GL/glut.h>
#include <GL/glext.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <iostream>
#include <string>
#include <vector>
#include <gmodule.h>
#include "interface.h"
#include "support.h"
#include <argp.h>
#include <assert.h>
#include "globalsE.h"
#include "Cube_P.h"
#include "utils.h"
#include "functions.h"
#include <glib.h>
#include "Configuration.h"
using namespace std;


/** Variables for the arguments.*/
const char *argp_program_version =
  "ascEditor 0.1";
const char *argp_program_bug_address =
  "<german.gonzalez@epfl.ch>";
/* Program documentation. */
static char doc[] =
  "ascEditor 0.1 program used to visualize 3D volume data and neurons in asc format";


/* A description of the arguments we accept. */
static char args_doc[] = "volume.nfo neuron.asc";

/* The options we understand. */
static struct argp_option options[] = {
  {"verbose"   ,  'v', 0, 0,  "Produce verbose output" },
  {"projection",  'p', 0, 0,
   "Toogles to maximum projection" },
  {"ascEditor",   'e', 0, 0, "Turns the ascEditor mode on"},
  {"selectEditor",   's', 0, 0, "Turns the selectEditor mode on"},
  {"alphaEditor",   'a', 0, 0, "Turns the alphaEditor mode on"},
  {"disp3DX",   'x', "value", 0, "Tells the X stage position"},
  {"disp3DY",   'y', "value", 0, "Tells the Y stage position"},
  {"disp3DZ",   'z', "value", 0, "Tells the Z stage position"},
  {"rot3DX",    'X', "value", 0, "Tells the X rotation"},
  {"rot3DY",    'Y', "value", 0, "Tells the Y rotation"},
  {"fullscreen",'f', 0, 0, "Turns full screen on"},
  {"screenshot",'S', "output_image", 0, "Takes an screenshot of the scene and exits"},
  {"configuration",'c', "scene.txt", 0, "reads the configuration file to set up the scene"},
  { 0 }
};

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *argments = (arguments*)state->input;

  switch (key)
    {
    case 'c':
      configurationFile = arg;
      break;
    case 'v':
      flag_verbose = 1;
      break;
    case 'p':
      flag_minMax = 1;
      break;
    case 'e':
      majorMode |= MOD_ASCEDITOR;
      break;
    case 's':
      majorMode |= MOD_SELECT_EDITOR;
      break;
    case 'a':
      majorMode |= MOD_ALPHA_EDITOR;
      break;
    case 'x':
      disp3DX = atof(arg);
      break;
    case 'y':
      disp3DY = atof(arg);
      break;
    case 'z':
      disp3DZ = atof(arg);
      break;
    case 'X':
      rot3DX = atof(arg);
      break;
    case 'Y':
      rot3DY = atof(arg);
      break;
    case 'f':
      flag_windowMaximize = true;
      break;
    case 'S':
      screenShotName = arg;
      majorMode = MOD_SCREENSHOT;
      break;
    case ARGP_KEY_ARG:
      objectNames.push_back(arg);
      break;

    case ARGP_KEY_END:
      /* Not enough arguments. */
      if (state->arg_num < 0)
        argp_usage (state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

void init_GUI()
{
  GtkComboBox* view_entry=GTK_COMBO_BOX(lookup_widget(GTK_WIDGET(ascEditor),"view_entry"));
  gtk_combo_box_set_active(view_entry,1);

  if(majorMode & MOD_SELECT_EDITOR)
    {
      GtkComboBox* selection_type=GTK_COMBO_BOX(lookup_widget(GTK_WIDGET(selectionEditor),"selection_type"));
      gtk_combo_box_set_active(selection_type,0);
    }

  static GtkTargetEntry drop_types[1] =
    {
      { "text/uri-list", GTK_TARGET_OTHER_APP, 0 }
    };

  gtk_drag_dest_set(drawing3D, GTK_DEST_DEFAULT_ALL, drop_types, 1, GDK_ACTION_COPY);
}

void
on_menu_plugins_submenu_activate                    (GtkMenuItem     *menuitem,
                                                     gpointer         user_data)
{
  /* string dir("plugins/bin/"); */
  string dir("lib/plugins/");

  char * pPath = getenv ("NESEG_PATH");
  if (pPath!=0)
    dir = string(pPath) + dir;

  GtkWidget *menu_label = gtk_bin_get_child(GTK_BIN(menuitem));
  const gchar* label = gtk_label_get_text(GTK_LABEL(menu_label));

  string sfile = dir+label;
  const char* plugin_name=(char*)sfile.c_str();

  plugin_run p_run;
  GModule *module = g_module_open (plugin_name, G_MODULE_BIND_LAZY);
  if (!module)
    {
      printf("Error while linking module %s\n", plugin_name);
    }
  else
    {
      if (!g_module_symbol (module, "plugin_run", (gpointer *)&p_run))
        {
          printf("Error while searching for symbol\n");
        }
      if (p_run == NULL)
        {
          printf("Symbol plugin_init is NULL\n");
        }
      else
        {
          vector<Object*> lObjects;
          lObjects.push_back(cube);

          /* for(vector< VisibleE* >::iterator itObj = toDraw.begin(); */
              /* itObj != toDraw.end(); itObj++ */
          for(int i = 0; i < toDraw.size(); i++)
            {
              lObjects.push_back(toDraw[i]);
              /* if((*itObj)->className()=="Image") */
                /* { */
                  /* Image< float >* img = (Image<float>*)*itObj; */
                  /* if(img!=0) */
                    /* { */
                      /* lObjects.push_back(img); */
                    /* } */
                  /* else */
                    /* printf("Null img\n"); */
                /* } */
            }

          p_run(lObjects); // execute init function
        }

      //Loads the key_pressed_symbol
      if (!g_module_symbol (module, "plugin_key_press_event",
                            (gpointer *)&p_key_press_event))
        {
          printf("Error while searching for symbol plugin_key_press_event\n");
        }
      if (p_key_press_event == NULL)
        {
          printf("Symbol p_key_press_event is NULL\n");
        }
      if (!g_module_symbol (module, "plugin_unproject_mouse",
                            (gpointer *)&p_unproject_mouse))
        {
          printf("Error while searching for symbol plugin_unproject_mouse\n");
        }
      if (p_unproject_mouse == NULL)
        {
          printf("Symbol p_unproject_mouse is NULL\n");
        }
      if (!g_module_symbol (module, "plugin_expose",
                            (gpointer *)&p_expose))
        {
          printf("Error while searching for symbol plugin_expose\n");
        }
      if (p_expose == NULL)
        {
          printf("Symbol p_expose is NULL\n");
        }
      if (!g_module_symbol (module, "plugin_motion_notify",
                            (gpointer *)&p_motion_notify))
        {
          printf("Error while searching for symbol plugin_motion_notify\n");
        }
      if (p_motion_notify == NULL)
        {
          printf("Symbol p_motion_notify is NULL\n");
        }

      /* if (!g_module_close (module)) */
        /* g_warning ("%s: %s", plugin_name, g_module_error ()); */
      plugin_init();
    }

}

void load_plugins()
{
  printf("*** Loading plugins\n");
  vector<string> files;
  /* string dir("plugins/bin"); */
  string dir("lib/plugins");

  char * pPath = getenv ("NESEG_PATH");
  if (pPath!=0)
    dir = string(pPath) + dir;

  int bRes = get_files_in_dir(dir, files);
  printf("Plugin directory : %s, Number of files : %d\n", dir.c_str(), files.size());
  if(bRes==0)
    {
      GtkMenuItem* menu_plugins=GTK_MENU_ITEM(lookup_widget(GTK_WIDGET(ascEditor),"menu_plugins"));

      GtkWidget *menu_plugins_ct = gtk_menu_new ();
      gtk_menu_item_set_submenu (menu_plugins, menu_plugins_ct);

      for(vector<string>::iterator itFile = files.begin();
          itFile != files.end();itFile++)
        {
          const char* filename=itFile->c_str();
          if(strcmp(filename,".")==0 || strcmp(filename,"..")==0)
            continue;

          string sfile = dir+"/";
          sfile += *itFile;
          filename=(char*)sfile.c_str();

          plugin_init p_init;
          GModule *module = g_module_open (filename, G_MODULE_BIND_LAZY);
          if (!module)
            {
              printf("Error while linking module %s\n", filename);
            }
          else
            {
              if (!g_module_symbol (module, "plugin_init", (gpointer *)&p_init))
                {
                  printf("Error while searching for symbol\n");
                }
              if (p_init == NULL)
                {
                  printf("Symbol plugin_init is NULL\n");
                }
              else
                {
                  p_init(); // execture init function

                  plugins.push_back(*itFile);
                  printf("Module %s has been loaded\n", filename);

                  GtkWidget *menu_plugins_submenu = gtk_menu_item_new_with_label(_(itFile->c_str()));
                  gtk_widget_show (menu_plugins_submenu);
                  gtk_container_add (GTK_CONTAINER (menu_plugins_ct), menu_plugins_submenu);

                  g_signal_connect ((gpointer) menu_plugins_submenu, "activate",
                                    G_CALLBACK (on_menu_plugins_submenu_activate),
                                    NULL);

                }

              /* if (!g_module_close (module)){ */
                /* char* nm = g_module_error (); */
                /* g_warning ("%i: %s", *itFile, nm ); */
              /* } */
            }
        }
    }
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

gboolean screenShotTimeOutExpited(gpointer data)
{
  if(majorMode != MOD_SCREENSHOT){
    return false;
  }
  if(screenShot_waitedEnough){
    saveScreenShot((char*)screenShotName.c_str());
    exit(0);
  }
  return true;
}


void loadConfiguration(Configuration* conf)
{
  printf("Loading the configuration from %s\n", conf->filename.c_str());

  string directoryConf = getDirectoryFromPath(conf->filename);
  conf->retrieveIfExists("disp3DX", &disp3DX);
  conf->retrieveIfExists("disp3DY", &disp3DY);
  conf->retrieveIfExists("disp3DZ", &disp3DZ);
  conf->retrieveIfExists("rot3DX", &rot3DX);
  conf->retrieveIfExists("rot3DY", &rot3DY);
  for(int i = 1; i < 10000; i++){
    bool success;
    string objectName;
    char buff[64];
    sprintf(buff, "obj%i", i);
    success = conf->retrieveIfExists(string(buff), &objectName);
    if(!success) break;
    else{
      printf("  adding key %s of object %s\n", buff, objectName.c_str());
      if(fileExists(objectName)){
        objectNames.push_back(objectName);
      }
      else if(fileExists(directoryConf+objectName))
        objectNames.push_back(directoryConf+objectName);
      else
        printf("I do not know anything about the file %s\n", objectName.c_str());
    }
  }
  string mod_display_string;
  conf->retrieveIfExists("mod_display", &mod_display_string);
  if(mod_display_string == "MOD_DISPLAY_3D")         mod_display = MOD_DISPLAY_3D;
  else if(mod_display_string == "MOD_DISPLAY_XY")    mod_display = MOD_DISPLAY_XY;
  else if(mod_display_string == "MOD_DISPLAY_XZ")    mod_display = MOD_DISPLAY_XZ;
  else if(mod_display_string == "MOD_DISPLAY_YZ")    mod_display = MOD_DISPLAY_YZ;
  else if(mod_display_string == "MOD_DISPLAY_COMBO") mod_display = MOD_DISPLAY_COMBO;
  else if(mod_display_string == "MOD_DISPLAY_DUAL")  mod_display = MOD_DISPLAY_DUAL;
  printf("  mod_display_string == %s\n", mod_display_string.c_str());
}



int
main (int argc, char *argv[])
{
  argcp = argc;
  argvp = argv;

  add_pixmap_directory("/usr/share/pixmaps/");

  //Initialization of the arguments
  flag_minMax = 0; //minIntensity
  flag_verbose = 0; // not verbose
  neuron_name = "";
  volume_name = "";
  majorMode   = MOD_VIEWER;

  argp_parse (&argp, argc, argv, 0, 0, 0);

  gtk_set_locale ();
  gtk_init (&argc, &argv);

  ascEditor = create_main_window ();

  //Adds opengl support to the drawing area
  GdkGLConfig *glconfig;
  gint major, minor;
  gdk_gl_query_version (&major, &minor);
  g_print ("GDK-OpenGL extension version - %d.%d\n",
           major, minor);
  glconfig = gdk_gl_config_new_by_mode ((GdkGLConfigMode)
                                        (GDK_GL_MODE_RGBA   |
                                         GDK_GL_MODE_DEPTH  |
                                         GDK_GL_MODE_DOUBLE));
  if(glconfig == NULL)
    std::cout << "Error creating the glconfig\n";

  drawing3D = lookup_widget(ascEditor, "drawing3D");

  gtk_widget_set_gl_capability (drawing3D,
                                glconfig,
                                NULL,
                                TRUE,
                                GDK_GL_RGBA_TYPE);

  cube = new Cube<uchar, ulong>();
  cube->dummy = true;

  glutInit(&argcp, argvp);
  if(flag_windowMaximize)
    gtk_window_maximize((GtkWindow*)ascEditor);
  if(configurationFile!=""){
    configuration = new Configuration(configurationFile);
    loadConfiguration(configuration);
  }


  gtk_widget_show (ascEditor);
  init_GUI();
  load_plugins();

  // Initialize glew

#ifdef WITH_GLEW
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    /* Problem: glewInit failed, something is seriously wrong. */
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
  }
  else {
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
  }
#endif

  init_GUI_late();

  if(majorMode & MOD_ASCEDITOR){
    GtkWidget* controlsAsc = create_ascEditControls();
    gtk_widget_show (controlsAsc);
  }
  if(majorMode & MOD_SELECT_EDITOR){
    selectionEditor = create_ascEditControlsSelect();
    gtk_widget_show (selectionEditor);
  }
  if(majorMode & MOD_ALPHA_EDITOR){
    alphaEditor = create_Alpha();
    gtk_widget_show (alphaEditor);
  }

  g_timeout_add(250, screenShotTimeOutExpited, NULL);

  gtk_main ();

  return 0;
}
