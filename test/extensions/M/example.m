%example for psfgen.m
% function is called with pairs of arguments:
% first is the name, second the value:
% lambda - wavelength of emission light [nm]
%   na - numerical aperture
%   pixelisze - size of the image pixel [nm]
%   sizevec - vector with number of pixels in X and Y direction
%   method    - 'airy' - airy disk (scalar approximation)
%             - 'gauss' - gaussian approximation [Zhang et al., 2007]
%   verbose - print out the parameters

% eg:
psf_odd = psfgen('lambda', 600, 'na', 1.4, 'pixelsize', 110, 'sizevec', [11 11], 'method', 'airy');

imagesc(psf_odd)
colormap(gray)
set (gca, 'DataAspectRatio', [1 1 1] );
title('psf - odd sizevec')
% generates 11x11 pixels image os PSF for 600nm emsion wavelength, 1.4NA objective with 110nm
% pixels-size using airy approximation of PSF (airy disk)
% for odd number of pixelsize PSF is placed in center. For even pixelsize
% PSF is centered on the upper-left pixel next to center...
psf_ev = psfgen('lambda', 600, 'na', 1.4, 'pixelsize', 110, 'sizevec', [10 10], 'method', 'airy');
figure
imagesc(psf_ev)
colormap(gray)
set (gca, 'DataAspectRatio', [1 1 1] );
title('psf - even sizevec')


%calling without some (or any) arguments generates pfs default values of
%missing arguments:
% eg:

psf_def = psfgen('lambda', 800, 'method', 'gauss');
figure
imagesc(psf_def)
colormap(gray)
set (gca, 'DataAspectRatio', [1 1 1] );
title('psf - default values')
% generates PSF with for 800nm emission wavelength, using Gaussian 
% approximation of the PSF and the rest of arguments will be set to default 
% values:

% default values:
% na = 1.2;
% pixelsize = 100; %nm
% sizevec = [25 25];


% All default values:
% lambda = 520; %nm
% na = 1.2;
% pixelsize = 100; %nm
% method = 'airy';
% sizevec = [25 25];

% or type
help psfgen 
