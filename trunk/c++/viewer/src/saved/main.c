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
#include "interface.h"
#include "support.h"
#include <argp.h>
#include <assert.h>
#include "globalsE.h"
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
  {"contourEditor",   'c', 0, 0, "Turns the contourEditor mode on"},
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
    case 'v':
      flag_verbose = 1;
      break;
    case 'p':
      flag_minMax = 1;
      break;
    case 'e':
      majorMode = MOD_ASCEDITOR;
      break;
    case 'c':
      majorMode = MOD_CONTOUREDITOR;
      break;

    case ARGP_KEY_ARG:
      objectNames.push_back(arg);
      break;

    case ARGP_KEY_END:
      /* Not enough arguments. */
      if (state->arg_num < 1)
        argp_usage (state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };


int
main (int argc, char *argv[])
{
  argcp = argc;
  argvp = argv;


  //Initialization of the arguments
  flag_minMax = 0; //minIntensity
  flag_verbose = 0; // not verbose
  neuron_name = "";
  volume_name = "";
  majorMode   = MOD_VIEWER;

  argp_parse (&argp, argc, argv, 0, 0, 0);

  gtk_set_locale ();
  gtk_init (&argc, &argv);

  ascEditor = create_ascEditor ();

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

  // Initialize glew
  glutInit(&argcp, argvp);
#ifdef WITH_GLEW
  int win = glutCreateWindow("GLEW Test");
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    /* Problem: glewInit failed, something is seriously wrong. */
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
  }
  else {
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
  }
  glutDestroyWindow(win);
#endif

  gtk_widget_show (ascEditor);

  if(majorMode == MOD_ASCEDITOR){
    GtkWidget* controlsAsc = create_ascEditControls();
    gtk_widget_show (controlsAsc);
  }
  else if(majorMode == MOD_CONTOUREDITOR){
    contourEditor = create_ascEditControlsContours();
    gtk_widget_show (contourEditor);
  }


  gtk_main ();
  return 0;
}
