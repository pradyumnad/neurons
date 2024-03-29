function A = spangles(I, angles, stride, edge_methods)
%SPEDGES computes spedge features along ANGLES
%
%   FEATURES = spedges(I, ANGLE, STRIDE, EDGE_METHODS)  computes spedge features on a 
%   grayscale image I at angles defined by ANGLES (given as a vector, eg.
%   [0 90 180 270]).  FEATURES contains a stack of images, each
%   corresponding to an angle in ANGLES. Each pixel in FEATURES(i,:,:) 
%   contains the distance to the nearest edge in the direction of 
%   ANGLES(i).  Edges are computed using Laplacian of Gaussian 
%   zero-crossings (!!! in the future we may add more methods for 
%   generating edges).  SIGMAS specifies the standard deviation(s) of the edge 
%   filter.  
%
%   Example:
%   -------------------------
%   I = imread('cameraman.tif');
%   angles = 0:30:330;
%   SPANGLE = spangles(I,angles,2, [29:34]);
%   figure; imagesc(squeeze(SPANGLE.spangles(1, 3, :, :))); axis image;
%
%   Copyright © 2008 Kevin Smith
%
%   See also SPEDGE_DIST, EDGE, VIEW_SPEDGES

A.angle = angles;
A.edgemethods = edge_methods;

for i = 1:length(angles)
    for s = 1:length(edge_methods)
        [A.spangles(i,s,:,:), A.spnorms(i,s,:,:), A.edge, A.G] = spangle_dist(I, angles(i), stride, edge_methods(s));   
    end
end
