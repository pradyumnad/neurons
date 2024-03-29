
/////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU General Public License         //
// version 2 as published by the Free Software Foundation.             //
//                                                                     //
// This program is distributed in the hope that it will be useful, but //
// WITHOUT ANY WARRANTY; without even the implied warranty of          //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// General Public License for more details.                            //
//                                                                     //
// Written and (C) by Aurelien Lucchi and Kevin Smith                  //
// Contact aurelien.lucchi (at) gmail.com or kevin.smith (at) epfl.ch  // 
// for comments & bug reports                                          //
/////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <map>
#include <string.h>
#include <cstdlib>
#include <sstream>
#include "enumerateLearners.h"

#define MIN_HEIGHT_VERT_HAAR 2
#define MIN_WIDTH_HORI_HAAR 2

// Haar-like feature parameters
struct sHA_params
{
  int step_size_x;
  int step_size_y;
};

enum eIT_KernelType { KERNEL_LINEAR, KERNEL_GAUSSIAN };

// Intensity Patch feature parameters
struct sIT_params
{
  int kernel_size;
  eIT_KernelType kernel_type; // Linear kernel, Gaussian kernel
};

vector<string> list_weak_learners;

int enumerate_learners(char **learner_type, int nb_learner_type, int max_width, int max_height, char**& weak_learners, int* weak_learner_type_indices)
{
  char temp[20];
  char* left_token = 0;
  char* right_token = 0;
  int size;
  int step_x = 1;
  int step_y = 1;
  int nb_learners = 0; // number of stored weak learners

  // Matlab passes width and height, not coordinates
  max_width++;
  max_height++;

  for(int iLearnerType = 0; iLearnerType<nb_learner_type;iLearnerType++) {

    left_token = learner_type[iLearnerType];
    if(learner_type[iLearnerType][0] == 'H' && learner_type[iLearnerType][1] == 'A')
      {
        // Generate Haar learners
        sHA_params params;
        while(left_token=strchr(left_token,'_'))
          {
            right_token=strchr(left_token+1,'_');
            // if null, point at the end of the string
            if(right_token == 0)
              right_token = learner_type[iLearnerType]+strlen(learner_type[iLearnerType]);

            size = right_token - left_token - 2;
            if(size > 20)
              {
                cout << "Error in enumerate_learner while parsing the string : incorrect format\n";
                return -1;
              }

            //printf("left_token %s\n",left_token);
            //printf("right_token %s\n",right_token);

            switch(*(left_token+1))
              {
              case 'x':
                strncpy(temp,left_token+2,size);
                temp[size] = 0;
                //printf("x %s\n",temp);
                params.step_size_x = atoi(temp);
                break;
              case 'y':
                strncpy(temp,left_token+2,size);
                temp[size] = 0;
                //printf("y %s\n",temp);
                params.step_size_y = atoi(temp);
                break;
              case 'u':
                strncpy(temp,left_token+2,size);
                temp[size] = 0;
                step_x = atoi(temp);
                break;
              case 'v':
                strncpy(temp,left_token+2,size);
                temp[size] = 0;
                step_y = atoi(temp);
                break;
              }
            left_token = right_token;
          }

        // Vertical learners
        for(int sx=1;sx<=max_width;sx+=params.step_size_x)
          {
            for(int sy=MIN_HEIGHT_VERT_HAAR;sy<=max_height;sy+=(params.step_size_y*MIN_HEIGHT_VERT_HAAR))
              {

                // Don't want to have weak learner of size 1
                if(sx == 1 && sy ==1)
                  continue;

                for(int ix=1;(ix+sx)<=max_width;ix+=step_x)
                  for(int iy=1;(iy+sy)<=max_height;iy+=step_y)
                    {
                      stringstream learner_id;
                      learner_id << learner_type[iLearnerType][0] << learner_type[iLearnerType][1]
                                 << "_Wax" << ix << "ay" << iy << "bx" << (ix + sx) << "by" << (iy + sy/2)
                                 << "_Bax" << ix << "ay" << (iy + sy/2) << "bx" << (ix + sx) << "by" << (iy + sy);
                      list_weak_learners.push_back(learner_id.str());
                    }
              }
          }

        // Horizontal learners
        for(int sx=MIN_WIDTH_HORI_HAAR;sx<=max_width;sx+=(params.step_size_x*MIN_WIDTH_HORI_HAAR))
          {
            for(int sy=1;sy<=max_height;sy+=params.step_size_y)
              {

                // Don't want to have weak learner of size 1
                if(sx == 1 && sy ==1)
                  continue;

                for(int ix=1;(ix+sx)<=max_width;ix+=step_x)
                  for(int iy=1;(iy+sy)<=max_height;iy+=step_y)
                    {
                      stringstream learner_id;
                      learner_id << learner_type[iLearnerType][0] << learner_type[iLearnerType][1]
                                 << "_Wax" << ix << "ay" << iy << "bx" << (ix + sx/2) << "by" << (iy + sy)
                                 << "_Bax" << (ix + sx/2) << "ay" << iy << "bx" << (ix + sx) << "by" << (iy + sy);
                      list_weak_learners.push_back(learner_id.str());
                    }
              }
          }

        // 3 elements - Vertical learners
        for(int sx=1;(3*sx)<=max_width;sx+=params.step_size_x)
          {
            for(int sy=1;sy<=max_height;sy+=params.step_size_y)
              {
                for(int ix=1;(ix+3*sx)<=max_width;ix+=step_x)
                  for(int iy=1;(iy+sy)<=max_height;iy+=step_y)
                    {
                      stringstream learner_id;
                      learner_id << learner_type[iLearnerType][0] << learner_type[iLearnerType][1]
                                 << "_Wax" << ix << "ay" << iy << "bx" << (ix + sx) << "by" << (iy + sy)
                                 << "_Bax" << (ix+sx) << "ay" << iy << "bx" << (ix + 2*sx) << "by" << (iy + sy)
                                 << "_Wax" << (ix+2*sx) << "ay" << iy << "bx" << (ix + 3*sx) << "by" << (iy + sy);
                      list_weak_learners.push_back(learner_id.str());
                    }
              }
          }

        // 2*2 elements
        int sy;
        for(int sx=1;(2*sx)<=max_width;sx+=params.step_size_x)
          {
            sy = sx;
            if((2*sy)<=max_height)
              {
                for(int ix=1;(ix+2*sx)<=max_width;ix+=step_x)
                  for(int iy=1;(iy+2*sy)<=max_height;iy+=step_y)
                    {
                      stringstream learner_id;
                      learner_id << learner_type[iLearnerType][0] << learner_type[iLearnerType][1]
                                 << "_Wax" << ix << "ay" << iy << "bx" << (ix + sx) << "by" << (iy + sy)
                                 << "_Bax" << (ix+sx) << "ay" << iy << "bx" << (ix + 2*sx) << "by" << (iy + sy)
                                 << "_Bax" << ix << "ay" << (iy+sy) << "bx" << (ix + sx) << "by" << (iy + 2*sy)
                                 << "_Wax" << (ix+sx) << "ay" << (iy+sy) << "bx" << (ix + 2*sx) << "by" << (iy + 2*sy);
                      list_weak_learners.push_back(learner_id.str());
                    }
              }
          }

        //weak_learner_type_indices[iLearnerType] = list_weak_learners.size() - nb_learners;
        weak_learner_type_indices[iLearnerType] = nb_learners;
        nb_learners += list_weak_learners.size();
      }

    if(learner_type[iLearnerType][0] == 'R' && learner_type[iLearnerType][1] == 'A')
      {
        //TODO : parse arguments
        int step_size_x = 20;
        int step_size_y = 20;

        for(int angle=0;angle<=360;angle+=30)
          {
            for(int edge_method=0;edge_method<22;edge_method++)
              {
                for(int x=1;x<=max_width;x+=step_size_x)
                for(int y=1;y<=max_height;y+=step_size_y)
                  {
                    string s;
                    stringstream out;
                    out << "RA_a" << angle << "_e" << edge_method << "_x" << x << "_y" << y;
                    s = out.str();
                    list_weak_learners.push_back(s);
                  }
              }
          }

        weak_learner_type_indices[iLearnerType] = nb_learners;
        nb_learners += list_weak_learners.size();
      }

    if(learner_type[iLearnerType][0] == 'I' && learner_type[iLearnerType][1] == 'T')
      {
        for(int i=0;i<6000;i++)
          {
            string s;
            stringstream out;
            out << "IT_" << i;
            s = out.str();

            list_weak_learners.push_back(s);
          }

        //weak_learner_type_indices[iLearnerType] = list_weak_learners.size() - nb_learners;
        weak_learner_type_indices[iLearnerType] = nb_learners;
        nb_learners += list_weak_learners.size();
      }
  }

  // Exporting the list
  weak_learners = new char*[list_weak_learners.size()];
  int idx = 0;
  for( vector<string>::iterator iter = list_weak_learners.begin();
       iter != list_weak_learners.end(); ++iter ) {
    weak_learners[idx] = new char[iter->length()+1];
    strcpy(weak_learners[idx], iter->c_str());
    idx++;
  }

  return list_weak_learners.size();
}
