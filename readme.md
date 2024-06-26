# patch.aul
## What is this
Plug-ins for fixing bugs or adding features in AviUtl or extension editing
The purpose is to undertake black magic

Read patch.aul.txt for more information

## About Build

- Prepare CUDA TOOLKIT (https://developer.nvidia.com/cuda-toolkit) )
We're going to use this OpenCL library Use the environment variable 'CUDA_PATH'

- Please prepare the `test` folder
This is a folder where you put `aviutl.exe` and so on. Debugging will run `aviutl.exe` in this directory

- Once built, release files will be gathered in the `pack` folder
