function [ tex ] = generate_texture( data, resolution, interp )
%GENERATE_TEXTURE Summary of this function goes here
%   Detailed explanation goes here

    [~, d] = size(data);

    if nargin < 2 || isempty(resolution)
        resolution = 128;
    end
    if nargin < 3 || isempty(interp)
        interp = 'nearest';
    end

    tex = zeros(resolution, resolution, d-2);
    
    Xd = 1 + data(:, 1) * (resolution-1);
    Yd = 1 + data(:, 2) * (resolution-1);
    [Xq, Yq] = meshgrid(1:resolution, 1:resolution);
    for i = 1:d-2
        tex(:, :, i) = reshape(...
            griddatan([Xd, Yd], data(:, 2+i), [Xq(:), Yq(:)], interp), ...
            resolution, resolution, [] ...
        );
    end

end

