function [I mv TMAX] = trkReadAndNormalizeImages( folder, MAX, STD)

disp('');

list = dir(fullfile(folder, '*.TIF'));
list2 = dir(fullfile(folder, '*.tif'));
if length(list) > length(list2)
    names = {list.name};
else
    names = {list2.name};
end
sorted_filenames = sort_nat(names);

TMAX = length(sorted_filenames);

I = cell(1,TMAX);
mv = I;


for t = 1:TMAX
    if mod(t,10) == 0
        fprintf('|');
    end
    
    filename = [folder sorted_filenames{t}];
    I{t} = imread( filename );

    Ilist = double(I{t}(:));
    
    % normalize according to the background;
    [h,x] = hist(Ilist,1000);
    hmax = max(h);
    h( h < .10 * hmax) = 0;
    minind = find(h > 0,1, 'first');
    maxind = find(h > 0,1, 'last');
    xmin = x(minind);
    xmax = x(maxind);
    Ilist(Ilist < xmin) = [];
    Ilist(Ilist > xmax) = [];
   
    % center the image background on zero
   	originalMedian = median(Ilist);
    originalStd = std(Ilist);
    
    % correct the image
    I{t} = I{t} - originalMedian;    
    I{t} = I{t} * (STD / originalStd);
    
    % create an 8-bit movie version of the image
    g = double(I{t});
    g(g < 0 )  = 0;
    g(g > MAX) = MAX;
    %g = g/MAX;
    g = g/(0.55 * MAX);  % for better visibility
    g(g > 1) = 1;
    
    g1 = g;
    g1(:,:,2) = g;
    g1(:,:,3) = g;
    
    mv{t} = g1;
end

fprintf('\n');
disp(['   loaded (' num2str(t) '/' num2str(TMAX) ') images from:  ' folder]);
disp('');

