clear; 
filename = 'violajones24x24.list';


Sigmas = [.5 1:8];
Kmin = 8;
Kmax = 16;
Afactor = 8;


IMSIZE = [24 24]; BLANK = zeros(IMSIZE);
fid=fopen(filename);
tline = fgetl(fid);

nfeatures = str2double(tline);

outname = ['sparseViolaJonesK' num2str(Kmin) '-' num2str(Kmax) '_' num2str(IMSIZE(1)) 'x' num2str(IMSIZE(2)) '.list'];

fid2 = fopen(outname, 'w');
fprintf(fid2, '%d\n', nfeatures);

disp(['...writing to ' outname]);

for i = 1:nfeatures
    tline = fgetl(fid);

    f = str2num(tline); %#ok<ST2NM>
    B = rectRender(f, IMSIZE, BLANK);
    [r,c] = find(B ~= 0);
    XC = mean(c)-1;
    YC = mean(r)-1;
    
    % select and appropriate number for K
    K = determineKfromMask(B, Kmin, Kmax, Afactor);
    
    % get the matching pursuit approximation
    [X Y W S m] = MatchingPursuitGaussianApproximation(B, Sigmas, K);
    X = X-1;
    Y = Y-1;

    

    n = length(X);
    f = [num2str(n) sprintf(' %f %f', XC, YC)];
    for k = 1:n        
        f = [f sprintf(' %f %f %d %d',  W(k), S(k), X(k), Y(k) )]; %#ok<*AGROW>
    end
    
    
    %keyboard;
    
    if mod(i, 200) == 0
        disp(['feature ' num2str(i) ' (K=' num2str(k) '/X=' num2str(numel(X)) ') samples: ' f]);
        R = sparseRender(str2num(f),IMSIZE); %#ok<ST2NM>
        subplot(1,2,1); imagesc(B); colormap gray;
        subplot(1,2,2); 
        cla; imagesc(R,[-max(abs(R(:))) max(abs(R(:)))]);  colormap gray; hold on;
        plot(X(W > 0)+1, Y(W > 0)+1, 'rs');
        plot(X(W < 0)+1, Y(W < 0)+1, 'gs'); 
        plot(XC+1,YC+1, 'mo'); hold off;
        drawnow;
    end
    
    fprintf(fid2, [f '\n']);

    
    %keyboard;
    
end

fclose(fid);
fclose(fid2);