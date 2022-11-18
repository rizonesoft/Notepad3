LoadPlugin("MPEG2DEC.dll") 
clip1 = Mpeg2Source("combine.d2v").Crop(16, 0, 688, 576).ConvertToRGB32
noise = AVISource("noise.avs").ConvertToRGB32
clip1 = clip1.Layer(noise, "add", 8, 0, 0, use_chroma=true)

clip2 = clip1.Trim(125766, 0)
clip = clip1.Layer(clip2, "add", 128, 0, 0, use_chroma=true).Trim(9180, \
    86780).AssumeFrameBased.ComplementParity.Bob

# build de-logo mask
logosrc = AviSource("nologo.avi").Weave.ConvertToRGB32.BilinearResize(59, 13)
masklogo = AviSource("whitelogo.avi").ConvertToRGB32
logo = logosrc.Mask(masklogo).BilinearResize(60, 27)

# now cover up that ugly white logo...
clip = clip.Layer(logo, "add", 255, 582, 36, use_chroma=true)

blur = clip.GeneralConvolution(0,
                 "10 10 10 10 10 
                 10 10 10 10 10
                 10 10 16 10 10
                 10 10 10 10 10
                 10 10 10 10 10")
# sobel edge detection... the power of the matrix!
hor = clip.GeneralConvolution(0,
                 "-1  0  1
                  -2  0  2
                  -1  0  1")
vert = clip.GeneralConvolution(0,
                 "-1  -2  -1
                  0  0  0
                  1  2  1")
edgemask = hor.Layer(vert,"lighten",255,0,0,1)

# now sharpen up just the edges of the blurred image...
clip = clip.Mask(edgemask)
clip = blur.Layer(clip, "add", 128, 0, 0)

return clip
