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

#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include "rays.h"

#define PI 3.1415926536

/* function [RAY1 RAY3 RAY4] = rays(E, G, angle, stride)
 * RAYS computes RAY features
 *   Example:
 *   -------------------------
 *   I = imread('cameraman.tif');
 *   SPEDGE = spedge_dist(I,30,2, 11);
 *   imagesc(SPEDGE);  axis image;
 *
 *
 *   FEATURE = spedge_dist(E, G, ANGLE, STRIDE)  computes a spedge 
 *   feature on a grayscale image I at angle ANGLE.  Each pixel in FEATURE 
 *   contains the distance to the nearest edge in direction ANGLE.  Edges 
 *   are computed using Laplacian of Gaussian zero-crossings (!!! in the 
 *   future we may add more methods for generating edges).  SIGMA specifies 
 *   the standard deviation of the edge filter.  
 */
void computeRays(const char *pImageName, double sigma, double angle)
{
    /*
    for(j=0;j<10;j++){
      printf("%d\n",pImage[j]);
      //pResult[j] = pIndices[j];
    }
    */
    
    /* Invert dimensions :
       Matlab : height, width
       OpenCV : width, hieght
    */
    /* Create output matrix */
    /*
    number_of_dims=mxGetNumberOfDimensions(prhs[0]);
    dim_array=mxGetDimensions(prhs[0]);
    const mwSize dims[]={dim_array[0],dim_array[1]};
    plhs[0] = mxCreateNumericArray(2,dims,mxUINT32_CLASS,mxREAL);
    pResult = (int*)mxGetData(plhs[0]);
    copyIntegralImage(pImage,dim_array[1],dim_array[0],pResult);
    */

  IplImage* img = cvLoadImage(pImageName,0);
  if(!img)
    {
      printf("Error while loading %s\n",pImageName);
      return;
    }


  //printf("%d %d\n",img->width, img->nChannels);
  
  IplImage* gx = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_16S, 1);
  IplImage* gy = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_16S, 1);
  //IplImage* g = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_32F, 1);
  IplImage* g = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 1);
  cvSobel(img, gx, 1, 0, 3);
  cvSobel(img, gy, 0, 1, 3);
  //cvSaveImage("gx.png",gx);
  //cvSaveImage("gy.png",gy);

  // compute the gradient norm GN
  int nx = 0;
  int ny = 0;
  int i = 0;
  uchar* ptrImg;
  for(int y=0;y<img->height;y++)
    for(int x=0;x<img->width;x++)
      {
        nx = ((short*)(gx->imageData + gx->widthStep*y))[x*gx->nChannels];
        nx *= nx;
        ny = ((short*)(gy->imageData + gy->widthStep*y))[x*gy->nChannels];
        ny *= ny;
        //ptrImg = (short*)&((short*)(g->imageData + g->widthStep*y))[x*g->nChannels];
        //ptrImg = &(((((float*)g->imageData) + g->widthStep*y)))[x*g->nChannels];
        //*ptrImg = 'a';
        ptrImg = ((uchar*)(g->imageData + g->widthStep*y)) + x*g->nChannels;
        //printf("%d %x\n",i++,ptrImg);
        //*ptrImg = 1;
        //*ptrImg = (uchar)(abs(nx)/4.0);
        *ptrImg = (uchar)(sqrt(nx+ny)/4.0f/1.5f);
      }
  printf("Saving\n");
  cvSaveImage("g.png",g);
  printf("Done\n");

  // ensure good angles
  angle = fmod(angle,360.0);
  if (angle < 0)
    angle += 360;
  
  // get a scamline in direction angle
  list<int> xs;
  list<int> ys;
  linepoints(img->width,img->height,angle,xs,ys);

  //for(int i = 0;i< xs.size(); i++)
  list<int>::iterator ix = xs.begin();
  list<int>::iterator iy = ys.begin();
  for(;ix != xs.end(); ix++,iy++)
    {
      printf("%d %d\n",*ix,*iy);
      cvSet2D(img,*iy,*ix,cvScalar(0));
    }
  cvSaveImage("img.png",img);

  printf("Releasing\n");
  cvReleaseImage(&img);
  cvReleaseImage(&gx);
  cvReleaseImage(&gy);
  cvReleaseImage(&g);
  printf("End release\n");
}

/*
 * defines the points in a line in an image at an arbitrary angle
 */
void linepoints(int img_width, int img_height, double angle, list<int>& xs, list<int>& ys)
{
  /*
  // flip the sign of the angle (matlab y axis points down for images) and
  // convert to radians
if Angle ~= 0
    %angle = deg2rad(Angle);
    angle = deg2rad(360 - Angle);
else
    angle = Angle;
end
  */

  // convert to radians
  angle = angle * (PI/180.0);

  /*
% format the angle so it is between 0 and less than pi/2
if angle > pi; angle = angle - pi; end
if angle == pi; angle = 0; end
  */

  // find where the line intercepts the edge of the image.  draw a line to
  // this point from (1,1) if 0<=angle<=pi/2.  otherwise pi/2>angle>pi draw 
  // from the upper left corner down.  linex and liney contain the points of 
  // the line
  int start_x;
  int start_y;
  int end_x;
  int end_y;
  /*
  int A_bottom_intercept[2][2];
  int A_right_intercept[2][2];
  int B_bottom_intercept[2];
  int B_right_intercept[2];
  */
  if ((angle >= 0 ) && (angle <= PI/2.0))
    {
      start_x = 0;
      start_y = 0;
      end_x = img_width-1;
      end_y = img_width*tan(angle)-1;

      intline(start_x, end_x, start_y, end_y, xs, ys, img_width, img_height);
    }
  else
    {
      start_x = 0;
      start_y = img_height-1;
      end_x = img_width-1;
      end_y = img_height*tan(angle)-1;

      intline(start_x, end_x, start_y, end_y, xs, ys, img_width, img_height);
    }

  printf("xy %f %d %d %d %d\n",angle,start_x, end_x, start_y, end_y);

  // if the angle points to quadrant 2 or 3, we need to re-sort the elements 
  // of Sr and Sc so they increase in the direction of the angle
  /*
if (270 <= Angle) || (Angle < 90)
    reverse_inds = length(Sr):-1:1;
    Sr = Sr(reverse_inds);
    Sc = Sc(reverse_inds);
end
  */
}


/*
% intline creates a line between two points
%INTLINE Integer-coordinate line drawing algorithm.
%   [X, Y] = INTLINE(X1, X2, Y1, Y2) computes an
%   approximation to the line segment joining (X1, Y1) and
%   (X2, Y2) with integer coordinates.  X1, X2, Y1, and Y2
%   should be integers.  INTLINE is reversible; that is,
%   INTLINE(X1, X2, Y1, Y2) produces the same results as
%   FLIPUD(INTLINE(X2, X1, Y2, Y1)).
 */
void intline(int x1, int x2, int y1, int y2, list<int>& xs, list<int>& ys,int img_width, int img_height)
{
  int x,y,t;
  int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);

  // Check for degenerate case.
  if ((dx == 0) && (dy == 0))
    {
      xs.push_back(x1);
      ys.push_back(y1);
      return;
    }

  bool flip = false;
  if (dx >= dy)
    {
      if (x1 > x2)
        {
          // Always "draw" from left to right.
          t = x1; x1 = x2; x2 = t;
          t = y1; y1 = y2; y2 = t;
          flip = true;
        }

      double m = (y2 - y1)/(double)(x2 - x1);
      printf("m %f\n",m);
      int y;
      for(int x = x1;x<=x2;x++)
        {
          y = round(y1 + m*(x - x1));
          if(x>=img_width || y >= img_height)
            break;
          xs.push_back(x);
          ys.push_back(y);
        }
    }
  else
    {
      if (y1 > y2)
        {
          // Always "draw" from bottom to top.
          t = x1; x1 = x2; x2 = t;
          t = y1; y1 = y2; y2 = t;
          flip = true;
        }
      double m = (x2 - x1)/(double)(y2 - y1);
      printf("m %f\n",m);
      int x;
      for(int y = y1;y<=y2;y++)
        {
          x = round(x1 + m*(y - y1));
          if(x>=img_width || y >= img_height)
            break;
          xs.push_back(x);
          ys.push_back(y);
        }
    }
  
  if (flip)
    {
      xs.reverse();
      ys.reverse();
    }
}

/*

    // compute the gradient norm GN
    MatrixN G(pGradient);
    double GN = sqrt(sum((G.^2),3));

% convert the Gradient G into unit vectors
G = gradientnorm(G);
v = double(v); eta = .00000001;
mag = sqrt(  sum( (v.^2),3)) + eta;
v = v ./ repmat(mag, [1 1 2]);


% get a scanline in direction angle
warning off MATLAB:nearlySingularMatrix; warning off MATLAB:singularMatrix;
[Sr, Sc] = linepoints(E,angle);
warning on MATLAB:nearlySingularMatrix; warning on MATLAB:singularMatrix;

% initialize the output matrices
RAY1 = zeros(size(E));  % distrance rays
RAY3 = zeros(size(E));  % gradient orientation
RAY4 = zeros(size(E));  % gradient norm

% determine the unit vector in the direction of the Ray
rayVector = unitvector(angle);

% if S touches the top & bottom of the image
if ((angle >= 45) && (angle <= 135))  || ((angle >= 225) && (angle <= 315))
    % SCAN TO THE LEFT!
    j = 0;
    c = Sc + j;
    inimage = find(c > 0);
    while ~isempty(inimage);
        r = Sr(inimage);
        c = Sc(inimage) + j;
        steps_since_edge = 0;  % the border of the image serves as an edge
        lastGN = 0;
        lastGA = 1;
        for i = 1:length(r);
            if E(r(i),c(i)) == 1
                steps_since_edge = 0;
                lastGA = rayVector * [G(r(i),c(i),1); G(r(i),c(i),2)]; 
                %if isnan(lastGA); keyboard; end;
                lastGN = GN(r(i),c(i));
            end
            RAY1(r(i),c(i)) = steps_since_edge;
            RAY3(r(i),c(i)) = lastGA;
            RAY4(r(i),c(i)) = lastGN;
            steps_since_edge = steps_since_edge +1;
        end
        j = j-1;
        c = Sc + j;
        inimage = find(c > 0);
    end


    % SCAN TO THE RIGHT!
    j = 1;
    c = Sc + j;
    inimage = find(c <= size(E,2));
    while ~isempty(inimage);
        r = Sr(inimage);
        c = Sc(inimage) + j;
        steps_since_edge = 0;  % the border of the image serves as an edge
        lastGN = 0;
        lastGA = 1;
        for i = 1:length(r);
            if E(r(i),c(i)) == 1
                steps_since_edge = 0;
                lastGA = rayVector * [G(r(i),c(i),1); G(r(i),c(i),2)]; 
                %if isnan(lastGA); keyboard; end;
                lastGN = GN(r(i),c(i));
            end
            RAY1(r(i),c(i)) = steps_since_edge;
            RAY3(r(i),c(i)) = lastGA;
            RAY4(r(i),c(i)) = lastGN;
            steps_since_edge = steps_since_edge +1;
        end
        j = j+1;
        c = Sc + j;
        inimage = find(c <= size(E,2));
    end
   
% if S touches left & right of image (-pi/4 > angle > pi/4) or (3pi/4 > angle > 5pi/4)
else
    % SCAN TO THE bottom!
    j = 0;
    r = Sr + j;
    inimage = find(r > 0);
    while ~isempty(inimage);
        r = Sr(inimage) + j;
        c = Sc(inimage);
        steps_since_edge = 0;  % the border of the image serves as an edge
        lastGN = 0;
        lastGA = 1;
        for i = 1:length(r);
            if E(r(i),c(i)) == 1
                steps_since_edge = 0;
                lastGA = rayVector * [G(r(i),c(i),1); G(r(i),c(i),2)]; 
                %if isnan(lastGA); keyboard; end;
                lastGN = GN(r(i),c(i));
            end
            RAY1(r(i),c(i)) = steps_since_edge;
            RAY3(r(i),c(i)) = lastGA;
            RAY4(r(i),c(i)) = lastGN;
            steps_since_edge = steps_since_edge +1;
        end
        j = j-1;
        r = Sr + j;
        inimage = find(r > 0);
    end


    % SCAN TO THE top!
    j = 1;
    r = Sr + j;
    inimage = find(r <= size(E,1));
    while ~isempty(inimage);
        r = Sr(inimage) + j;
        c = Sc(inimage);
        steps_since_edge = 0;  % the border of the image serves as an edge
        lastGN = 0;
        lastGA = 1;
        for i = 1:length(r);
            if E(r(i),c(i)) == 1
                steps_since_edge = 0;
                lastGA = rayVector * [G(r(i),c(i),1); G(r(i),c(i),2)]; 
                %if isnan(lastGA); keyboard; end;
                lastGN = GN(r(i),c(i));
            end
            RAY1(r(i),c(i)) = steps_since_edge;
            RAY3(r(i),c(i)) = lastGA;
            RAY4(r(i),c(i)) = lastGN;
            steps_since_edge = steps_since_edge +1;
        end
        j = j+1;
        r = Sr + j;
        inimage = find(r <= size(E,1));
    end
end

if stride ~= 1
    RAY1 = RAY1(1:stride:size(RAY1,1), 1:stride:size(RAY1,2));
    RAY3 = RAY3(1:stride:size(RAY3,1), 1:stride:size(RAY3,2));
    RAY4 = RAY4(1:stride:size(RAY4,1), 1:stride:size(RAY4,2));
end








function v = gradientnorm(v)
v = double(v); eta = .00000001;
mag = sqrt(  sum( (v.^2),3)) + eta;
v = v ./ repmat(mag, [1 1 2]);

*/
