# patch.aul
## What is this
Plug-ins for fixing bugs or adding features in AviUtl or extension editing
The purpose is to undertake black magic

Read patch.aul.txt for more information

## Installation Method
1. Access https://github.com/AviUtl-EN-Translation/patch.aul_en/releases/latest.

![image](https://github.com/AviUtl-EN-Translation/auls_addshortcut_en/assets/173457541/6e5e8f8b-d7be-4d47-9168-1f0a9a453c64)



2. Click on patch.aul_rXX_ss_N.zip to download.



3. Continue if a download warning appears in your browser (the warning will disappear as more people download it).
3-1. It may be detected by antivirus software (reported by McAfee). [Plug-in detected by antivirus software]

<img src="https://github.com/AviUtl-EN-Translation/auls_addshortcut_en/assets/173457541/0f369f71-31c5-4f21-9fef-c347d46e2831" width="600px" height=auto title="aul Path" alt="patch.aul Path Image"></img>

4. Open the downloaded zip file and move the patch.aul,EnMod1_5.aul and EnMod_1_5XP.aul file to the folder containing aviutl.exe.
5. 
![image](https://github.com/AviUtl-EN-Translation/auls_addshortcut_en/assets/173457541/7824f4c5-dd38-40f3-bb44-f2827488da4d)

![image](https://github.com/AviUtl-EN-Translation/patch.aul_en/assets/173457541/c0a7ea17-3835-4e4b-9e49-604a8a79faf3)

7. If "Info" > patch.aul exists in the top menu of AviUtl and patch.aul info appears when clicked the installation is successful.
   - If not, check the installation of the [Visual C++ Redistributable Package 2015-2022](https://scrapbox.io/nazosauna/Visual_C++_%E5%86%8D%E9%A0%92%E5%B8%83%E5%8F%AF%E8%83%BD%E3%83%91%E3%83%83%E3%82%B1%E3%83%BC%E3%82%B8%E3%82%92%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%88%E3%83%BC%E3%83%AB%E3%81%99%E3%82%8B).
   - If it still doesn't appear, check [the current location of the opened aviutl.exe](https://scrapbox.io/nazosauna/%E7%8F%BE%E5%9C%A8%E9%96%8B%E3%81%84%E3%81%A6%E3%81%84%E3%82%8Baviutl.exe%E3%81%AE%E5%A0%B4%E6%89%80%E3%82%92%E7%A2%BA%E8%AA%8D%E3%81%99%E3%82%8B).
8. There are a few cautions
   - Install the latest L-SMASH Works and InputPipePlugin and confirm their operation. [Update L-SMASH Works to the latest version](https://scrapbox.io/nazosauna/L-SMASH_Works%E3%82%92%E6%9C%80%E6%96%B0%E7%89%88%E3%81%AB%E6%9B%B4%E6%96%B0%E3%81%99%E3%82%8B)
   - If a message appears at startup, refer to [If a message appears at startup after installing patch.aul](https://scrapbox.io/nazosauna/patch.aul%E5%B0%8E%E5%85%A5%E5%BE%8C_%E8%B5%B7%E5%8B%95%E6%99%82%E3%81%AB%E3%83%A1%E3%83%83%E3%82%BB%E3%83%BC%E3%82%B8%E3%81%8C%E5%87%BA%E3%82%8B).
   - [Examples of problems that occur after installing patch.aul and how to handle them
](https://scrapbox.io/nazosauna/patch.aul%E5%B0%8E%E5%85%A5%E5%BE%8C%E3%81%AB%E5%95%8F%E9%A1%8C%E3%81%8C%E8%B5%B7%E3%81%93%E3%81%A3%E3%81%9F%E4%BE%8B%E3%81%A8%E5%AF%BE%E5%87%A6)

9. Assign the shortcut key "Redo" (since the default is not added if you have previously saved shortcut keys).
   ![image](https://github.com/AviUtl-EN-Translation/patch.aul_en/assets/173457541/468a9c78-8eee-427a-a06a-a8b4d1e7ecd8)
   - The default is Ctrl+Y.

10. Increase the cache size in [System Settings](https://scrapbox.io/nazosauna/%E3%82%B7%E3%82%B9%E3%83%86%E3%83%A0%E3%81%AE%E8%A8%AD%E5%AE%9A) if it is small (because the number of functions using this cache has increased).
   - Half to three-quarters of the physical memory size installed on the PC is appropriate. [/aviutl/Setup](https://scrapbox.io/aviutl/%E3%82%BB%E3%83%83%E3%83%88%E3%82%A2%E3%83%83%E3%83%97)
   - The size of the physical memory can be checked in Task Manager, etc. (1GB = 1024MBytes).

11. Review the [Image Data Cache Number](https://scrapbox.io/nazosauna/%E7%94%BB%E5%83%8F%E3%83%87%E3%83%BC%E3%82%BF%E3%81%AE%E3%82%AD%E3%83%A3%E3%83%83%E3%82%B7%E3%83%A5%E6%95%B0) in [Advanced Editing Settings](https://scrapbox.io/nazosauna/%E6%8B%A1%E5%BC%B5%E7%B7%A8%E9%9B%86%E3%81%AE%E7%92%B0%E5%A2%83%E8%A8%AD%E5%AE%9A) (because increasing it reduces the memory error-prone cache). [Recommended settings to avoid memory-related errors](https://scrapbox.io/nazosauna/%E3%83%A1%E3%83%A2%E3%83%AA%E9%96%A2%E4%BF%82%E3%82%A8%E3%83%A9%E3%83%BC%E3%82%92%E8%B5%B7%E3%81%93%E3%81%95%E3%81%AA%E3%81%84%E3%81%9F%E3%82%81%E3%81%AE%E6%8E%A8%E5%A5%A8%E8%A8%AD%E5%AE%9A)
    - If the image becomes strange when using scenes or scripts, increase it by 1. [Problems related to Image Data Cache Number](https://scrapbox.io/nazosauna/%E7%94%BB%E5%83%8F%E3%83%87%E3%83%BC%E3%82%BF%E3%81%AE%E3%82%AD%E3%83%A3%E3%83%83%E3%82%B7%E3%83%A5%E6%95%B0%E3%81%AB%E9%96%A2%E3%81%99%E3%82%8B%E3%83%88%E3%83%A9%E3%83%96%E3%83%AB)



## About Build

- Prepare CUDA TOOLKIT (https://developer.nvidia.com/cuda-toolkit) )
We're going to use this OpenCL library Use the environment variable 'CUDA_PATH'

- Please prepare the `test` folder
This is a folder where you put `aviutl.exe` and so on. Debugging will run `aviutl.exe` in this directory

- Once built, release files will be gathered in the `pack` folder
