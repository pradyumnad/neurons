#include "Cube_C.h"

Cube_C::Cube_C(string filenameParams) : Cube_P()
{
  v_r = 1.0;
  v_g = 1.0;
  v_b = 1.0;

  string extension = getExtension(filenameParams);
  directory = getDirectoryFromPath(filenameParams);
  if(extension=="tiff" || extension=="TIFF" ||
     extension=="tif" || extension=="TIF"){
    loadFromTIFFImage(filenameParams);
  }
  else{
    printf("Loading cube_C\n");
    std::ifstream file(filenameParams.c_str());
    if(!file.good())
      printf("Cube_C::load_parameters: error loading the file %s\n", filenameParams.c_str());

    string name;
    string attribute;
    while(file.good())
      {
        file >> name;
        file >> attribute;
        if(!strcmp(name.c_str(), "filenameVoxelDataR"))
          filenameVoxelDataR = attribute;
        else if(!strcmp(name.c_str(), "filenameVoxelDataG"))
          filenameVoxelDataG = attribute;
        else if(!strcmp(name.c_str(), "filenameVoxelDataB"))
          filenameVoxelDataB = attribute;
        else if(!strcmp(name.c_str(), "type"))
          type = attribute;
        else
          printf("Cube_C::load_parameters: Attribute %s and value %s not known\n",
                 name.c_str(), attribute.c_str());
      }
    if(type != "color"){
      printf("Cube_C called to load an nfo file that is not a Cube_C... exiting\n");
      exit(0);
    }
    if ( (filenameVoxelDataR == "") || (filenameVoxelDataG == "") ||
         (filenameVoxelDataB == "") )
      {
        printf("Cube_C one of the color channels is not defined... exiting\n");
        exit(0);
      }
    data.resize(0);
    if(fileExists(filenameVoxelDataR))
      data.push_back(new Cube<uchar, ulong>(filenameVoxelDataR));
    else if (fileExists(directory + "/" + filenameVoxelDataR))
      data.push_back(new Cube<uchar, ulong>(directory + "/" + filenameVoxelDataR));
    if(fileExists(filenameVoxelDataG))
      data.push_back(new Cube<uchar, ulong>(filenameVoxelDataG));
    else if (fileExists(directory + "/" + filenameVoxelDataG))
      data.push_back(new Cube<uchar, ulong>(directory + "/" + filenameVoxelDataG));
    if(fileExists(filenameVoxelDataB))
      data.push_back(new Cube<uchar, ulong>(filenameVoxelDataB));
    else if (fileExists(directory + "/" + filenameVoxelDataB))
      data.push_back(new Cube<uchar, ulong>(directory + "/" + filenameVoxelDataB));
  } //not .tiff

  printf("Now all the cubes should be loaded -> %i\n", data.size());

  this->cubeHeight  = data[0]->cubeHeight;
  this->cubeDepth   = data[0]->cubeDepth;
  this->cubeWidth   = data[0]->cubeWidth;
  this->voxelHeight = data[0]->voxelHeight;
  this->voxelDepth  = data[0]->voxelDepth;
  this->voxelWidth  = data[0]->voxelWidth;
  // this->r_max       = data[0]->r_max;
  // this->s_max       = data[0]->s_max;
  // this->t_max       = data[0]->t_max;
  // this->nColToDraw  = data[0]->nColToDraw;
  // this->nRowToDraw  = data[0]->nRowToDraw;
  this->filenameVoxelData = "";
  this->directory          = getDirectoryFromPath(filenameParams);
  this->filenameParameters = getNameFromPath(filenameParams);
  this->type               = "color";


  // glGenTextures(1, &wholeTexture);

  // this->x_offset = data[0]->x_offset;
  // this->y_offset = data[0]->y_offset;
  // this->z_offset = data[0]->z_offset;
}

void Cube_C::loadFromTIFFImage(string imageName)
{
  //First we need some information
  int dircount = 0;
  uint32 w; uint32 h; uint32 depth;
  uint16 bps, samplesPerPixel; 
  TIFF* tif = TIFFOpen(imageName.c_str(), "r");
  if(!tif){
    printf("Cube_C::Error getting the tiff image.\n");
    exit(0);
  } else {
    do{
        dircount++;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
        TIFFGetField(tif, TIFFTAG_IMAGEDEPTH, &depth);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
      } while (TIFFReadDirectory(tif));
  }
  TIFFClose(tif);
  printf("Cube_C::The tiff has %i layers of size=[%i,%i] and %i bits per sample\n"
         "depth = %i, samplesPerPixel=%i\n",
         dircount, w, h, bps, depth, samplesPerPixel);
  data.resize(0);
  data.push_back(new Cube<uchar, ulong>(w,h,dircount,1,1,1));
  data.push_back(new Cube<uchar, ulong>(w,h,dircount,1,1,1));
  data.push_back(new Cube<uchar, ulong>(w,h,dircount,1,1,1));

  // exit(0);

  //Now we fill the cubes
  tif = TIFFOpen(imageName.c_str(), "r");
  uint32 px;
  uint32 mask = 0x000000FF;
  uint32* raster;
  printf("Loading the layers[");
  for(int z = 0; z < dircount; z++){
    //Prepare the raster
    raster = (uint32*) _TIFFmalloc(w*h * sizeof (uint32));
    if (TIFFReadRGBAImage(tif, w, h, raster, 0)) {
      for(int y = 0; y < h; y++){
        for(int x = 0; x < w; x++){
          px = raster[y*w+x];
          data[0]->put(x,h-1-y,z, (uchar)32*(mask & px));
          data[1]->put(x,h-1-y,z, (uchar)32*(mask & px >> 8));
          data[2]->put(x,h-1-y,z, (uchar)32*(mask & px >> 16));
        }//x
      }//y
      TIFFReadDirectory(tif);
      printf("#"); fflush(stdout);
    }//TIFFread
    else{
      printf("Cube_C::loadFromTIFFImage::error loading the raster for directory %i\n", z);
    }
    _TIFFfree(raster);
  }
  TIFFClose(tif);
  printf("]\n");
}


void Cube_C::load_texture_brick(int row, int col, float scale, float _min, float _max)
{
  if(!glIsTexture(wholeTexture)){
    glGenTextures(1, &wholeTexture);
    printf("Cube_C::generated texture out of nowhere %i\n", wholeTexture);
  } else {
    glDeleteTextures(1, &wholeTexture);
    glGenTextures(1, &wholeTexture);
    printf("Cube_C::generated texture by destroying another one %i\n", wholeTexture);
  }
  printf("Cube_C, loading texture brick\n");
  nColToDraw = col;
  nRowToDraw = row;
  GLint max_texture_size = 0;
#ifdef WITH_GLEW
  glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max_texture_size);
  max_texture_size = D_MAX_TEXTURE_SIZE;
#endif
  max_texture_size = D_MAX_TEXTURE_SIZE;
  printf("Max texture size %i\n", max_texture_size);

  int limit_x = min((int)cubeWidth,
                    min((int)max_texture_size,
                        (int)cubeWidth - (nColToDraw*max_texture_size)));
  int limit_y = min((int)cubeHeight,
                    min((int)max_texture_size,
                        (int)cubeHeight - (nRowToDraw*max_texture_size)));
  int limit_z =  (int)min(max_texture_size, (int)cubeDepth);

  int texture_size_x = (int) pow(2.0, ceil(log((double)limit_x)/log(2.0)) );
  int texture_size_y = (int) pow(2.0, ceil(log((double)limit_y)/log(2.0)) );
  int texture_size_z = (int) pow(2.0, ceil(log((double)limit_z)/log(2.0)) );

  //Limit od the textures. They are object variables
  r_max = (double)limit_x/texture_size_x;
  s_max = (double)limit_y/texture_size_y;
  t_max = (double)limit_z/texture_size_z;

  printf("Load_texture_brick: texture size %i, limit_x = %i, limit_y = %i limit_z = %i\n               texture_size: x=%i y=%i z=%i, r_max=%f, s_max=%f, t_max=%f\n",
         max_texture_size, limit_x, limit_y, limit_z,
         texture_size_x, texture_size_y,texture_size_z,
         r_max,s_max,t_max);


  if( (limit_x<0) || (limit_y<0))
    {
      printf("Cube_C::load_texture_brick requested col %i row %i out of range"
             ", loading 0,0\n", nColToDraw, nRowToDraw);
      nColToDraw = 0;
      nRowToDraw = 0;
      limit_x = min(max_texture_size,(int)cubeWidth);
      limit_y = min(max_texture_size,(int)cubeHeight);
    }
  printf("Cube_C::load_texture_brick: Whole texure = %i\n", wholeTexture);
// #ifdef WITH_GLEW
  // glEnable(GL_TEXTURE_3D);
  // glBindTexture(GL_TEXTURE_3D, wholeTexture);
// #endif
  int col_offset = (col)*max_texture_size;
  int row_offset = (row)*max_texture_size;

  int size_texture = max_texture_size;
  GLubyte voxel;

  int texture_size;
  texture_size = texture_size_x*texture_size_y*texture_size_z*3;
  GLubyte* texels =(GLubyte*)(malloc(texture_size*sizeof(GLubyte)));
  for(int t = 0; t < texture_size; t++)
    texels[t] = 0;

  int depth_y;
  int depth_z;
  float R = 0; float G = 0; float B = 0;
  for(int z = 0; z < limit_z; z++)
    {
      depth_z = z*texture_size_x*texture_size_y*3;
      for(int y = 0; y < limit_y; y++)
        {
          depth_y = y*texture_size_x*3;
          for(int x = 0; x < limit_x; x++)
            {
              voxel = (GLubyte)data[0]->at(col_offset+x,row_offset+y,z)*scale;
              texels[depth_z + depth_y + 3*x + 0] = voxel;
              R+= voxel;
              voxel = (GLubyte)data[1]->at(col_offset+x,row_offset+y,z)*scale;
              texels[depth_z + depth_y + 3*x + 1] = voxel;
              G+= voxel;
              voxel = (GLubyte)data[2]->at(col_offset+x,row_offset+y,z)*scale;
              texels[depth_z + depth_y + 3*x + 2] = voxel;
              B+= voxel;
            }
        }
      printf("#");
      fflush(stdout);
    }
  printf("] average values are [R,G,B] = [%f,%f,%f]\n", R/(limit_x*limit_y*limit_z),
         G/(limit_x*limit_y*limit_z),B/(limit_x*limit_y*limit_z));
  printf("wholeTexture = %i\n", wholeTexture);

#ifdef WITH_GLEW
  glEnable(GL_TEXTURE_3D);
  glBindTexture(GL_TEXTURE_3D, wholeTexture);
  glTexParameteri(GL_TEXTURE_3D,  GL_TEXTURE_MIN_FILTER, D_TEXTURE_INTERPOLATION);
  glTexParameteri(GL_TEXTURE_3D,  GL_TEXTURE_MAG_FILTER, D_TEXTURE_INTERPOLATION);
  glTexParameteri(GL_TEXTURE_3D,  GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_3D,  GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_3D,  GL_TEXTURE_WRAP_R, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_3D,  GL_TEXTURE_PRIORITY, 1.0);
  GLfloat border_color[4];
  for(int i = 0; i < 4; i++)
    border_color[i] = 1.0;
  glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, border_color);
  glEnable(GL_TEXTURE_3D);
  glBindTexture(GL_TEXTURE_3D, wholeTexture);
  glTexImage3D(GL_TEXTURE_3D,
               0, // level
               GL_RGB,
               texture_size_x, texture_size_y, texture_size_z,
               0, //border
               GL_RGB,
               GL_UNSIGNED_BYTE,
               texels);
  GLenum error = glGetError();
  if(error != GL_NO_ERROR){
    printf("Cube_C there has been an error loading the texture\n");
    char* error_msg = (char *) gluErrorString(error);
    printf("The error is %s\n", error_msg);
  } else {
    printf("No error loading the texture\n");
  }

#endif
  free(texels);

  // glBindTexture(GL_TEXTURE_3D, wholeTexture);
}


void Cube_C::print_size()
{
}

void Cube_C::min_max(float* min, float* max){}

Cube_P* Cube_C::threshold(float thres, string outputName,
                  bool putHigherValuesTo, bool putLowerValuesTo,
                  float highValue, float lowValue){}

void Cube_C::print_statistics(string name){}

void Cube_C::histogram(string name){}

void Cube_C::save_as_image_stack(string dirname){}

vector< vector< int > > Cube_C::decimate
(float threshold, int window_xy, int window_z, string filemane,
 bool save_boosting_response){}

vector< vector< int > > Cube_C::decimate_log
(float threshold, int window_xy, int window_z, string filemane,
 bool save_boosting_response){}

/** Produces a vector of the NMS in the layer indicated.*/
vector< vector< int > > Cube_C::decimate_layer
(int nLayer, float threshold, int window_xy, string filename){}

void Cube_C::allocate_alphas(int ni, int nj, int nk){}

void Cube_C::delete_alphas(int ni, int nj, int nk){}


float Cube_C::get(int x, int y, int z){ return 0.0;}
