/*
	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include "util_int.hpp"

namespace OFS {
	namespace AviUtl {
		constexpr i32 InitAuf = 0x02c930;
		constexpr i32 VersionString = 0x07425c;
		constexpr i32 default_resource_hmod = 0x2c525c;
		constexpr i32 getsys_versionstr_arg = 0x022187;
		constexpr i32 current_resource_hmod = 0x2d910c;
		constexpr i32 filter_n = 0x258d04;
		constexpr i32 edit_handle_ptr = 0x08717c;
		constexpr i32 saveProjectFile = 0x024160;
		constexpr i32 exfunc = 0x0a8c78;


		constexpr i32 ini_shiftselect = 0x086398;

		constexpr i32 get_exe_dir = 0x02c8c0;

		constexpr i32 filter_change_size_ptr = 0x0814e8;
		constexpr i32 filter_change_framerate_ptr = 0x080b28;
		constexpr i32 filter_clipping_and_resize_ptr = 0x07ad58;

		constexpr i32 str_AviUtl = 0x0742a0; // "AviUtl"
		

		constexpr i32 str_dot_avi = 0x0745fc; // ".avi"
		constexpr i32 str_asterisk_dot_aul = 0x075a10; // "*.aul"
		constexpr i32 str_asterisk_dot_auc = 0x0748fc; // "*.auc"
		constexpr i32 str_asterisk_dot_aui = 0x07498c; // "*.aui"
		constexpr i32 str_asterisk_dot_auf = 0x0752fc; // "*.auf"
		constexpr i32 str_asterisk_dot_auo = 0x0750d4; // "*.auo"

		constexpr i32 str_percent_s_percent_s = 0x0743c8; // "%s%s"
		constexpr i32 str_percent_s_percent_s_percent_s = 0x0743e8; // "%s%s"

		constexpr i32 str_plugins_backslash = 0x074904; // "plugins\\"


	}

	namespace ExEdit {
		constexpr i32 exedit_fp = 0x14d4b4;
		
		constexpr i32 aviutl_hwnd = 0x135c6c;
		constexpr i32 exedit_hwnd = 0x177a44;
		constexpr i32 settingdialog_hwnd = 0x1539c8;
		

		constexpr i32 tl_title = 0x0a4cfc;

		constexpr i32 blend_yca_normal_func = 0x007df0;
		constexpr i32 blend_yc_normal_func = 0x007f20;
		
		constexpr i32 rendering_mt_func = 0x078140;
		constexpr i32 calc_xzuv_offset_range = 0x07c050;
		constexpr i32 push_rendering_data = 0x07c3f0;
		constexpr i32 rendering_data_array = 0x1ec890;
		constexpr i32 rendering_data_count = 0x1ec728;
		constexpr i32 rendering_inc_or_dec = 0x0ade4c;
		constexpr i32 cam_screen_d16 = 0x1ec858;
		constexpr i32 z_offset_default = 0x1e48f8;

		constexpr i32 ConvertFilter2Exo_TrackScaleJudge_RangeBegin = 0x028a8d;
		constexpr i32 ConvertFilter2Exo_TrackScaleJudge_Overwrite1 = 0x028a84;
		constexpr i32 ConvertFilter2Exo_TrackScaleJudge_Overwrite2 = 0x0289cb;
		constexpr i32 ConvertFilter2Exo_TrackScaleJudge_Overwrite3 = 0x028a00;

		constexpr i32 efSceneAudio_exdatause_idx_type = 0x0aeb5c;
		constexpr i32 efSceneAudio_exdatause_idx_name = 0x0aeb60;
		constexpr i32 efScene_exdatause_idx_name = 0x0ae9b0;

		constexpr i32 exo_readtrack = 0x029090;
		constexpr i32 str2int2 = 0x0918ab;

		constexpr i32 exo_trackparam_overwrite = 0x0299d1;

		constexpr i32 efpip_g = 0x1b2b20;

		constexpr i32 getpixeldata = 0x09a65c;
		constexpr i32 rgb2yc = 0x06fed0;
		constexpr i32 yc_conv_w_loop_count = 0x1e4300;

		constexpr i32 yc_vram_w = 0x149840;
		constexpr i32 yc_vram_h = 0x14ca4c;
		constexpr i32 yc_max_w = 0x178ec0;
		constexpr i32 yc_max_h = 0x1790c8;
		constexpr i32 yc_vram_linesize = 0x158d10;
		
		constexpr i32 timeline_obj_click_mode = 0x177a24;

		constexpr i32 exedit_buffer_size = 0x177a28;
		constexpr i32 exedit_max_h_add8 = 0x135c64;
		constexpr i32 exedit_buffer_line = 0x135c68;

		constexpr i32 exedit_max_w = 0x196748;
		constexpr i32 exedit_max_h = 0x1920e0;

		constexpr i32 memory_ptr = 0x1a5328;
		constexpr i32 zbuffer_ptr = 0x1ec7ac;
		constexpr i32 zbuffer_size = 0x1e4900;
		constexpr i32 clear_zbuffer = 0x07d320;

		constexpr i32 shadowmap_ptr = 0x1e9ea8;
		constexpr i32 shadowmap_size = 0x1ec888;
		constexpr i32 clear_shadowmap = 0x07d390;

		constexpr i32 camera_mode = 0x013596c;
		constexpr i32 z_order_ofi = 0x134ed0;
		constexpr i32 z_order_coord = 0x135110;
		constexpr i32 camera_dlg_visible = 0x1359b4;
		constexpr i32 select_camera_ofi = 0x1359ec;
		constexpr i32 camera_value_change_mode = 0x135a80;
		

		constexpr i32 fast_process = 0x2308a0;
		constexpr i32 is_saving = 0x1a52e4;

		constexpr i32 CreateFontBorder_var_ptr = 0x1a6bb0;
		constexpr i32 efTextShadowBorderComboHWND_ptr = 0x23635c;

		constexpr i32 CreateFigure_var_ptr = 0x1e4798;
		constexpr i32 CreateFigure_circle_func_call = 0x073882;
		constexpr i32 CreateFigure_circle_func_mt_call = 0x0738ac;
		constexpr i32 CreateFigure_polygons_func_call = 0x073949;
		constexpr i32 CreateFigure_polygons_func_mt_call = 0x07395b;

		constexpr i32 efBorder_func_proc_var_ptr = 0x0a5d28;
		constexpr i32 efBorder_func_proc_ptr = 0x0515d0;
		constexpr i32 efBorder_var_ptr = 0x1b1e30;
		constexpr i32 efBlur_func_proc_ptr = 0x0a0750;
		constexpr i32 efBlur_var_ptr = 0x11ec34;
		constexpr i32 efGlow_var_ptr = 0x1b2010;
		constexpr i32 efPolarTransform_func_proc = 0x0748a0;
		constexpr i32 efPolarTransform_func_proc_ptr = 0x0add30;
		constexpr i32 efPolarTransform_mt_func_call = 0x074a62;
		constexpr i32 efPolarTransform_var_ptr = 0x1e48c0;
		constexpr i32 efDisplacementMap_mt_func_call = 0x01f154;
		constexpr i32 efDisplacementMap_var_ptr = 0x11effc;
		constexpr i32 efRadiationalBlur_func_proc = 0x00b310;
		constexpr i32 efRadiationalBlur_func_proc_ptr = 0x09fdb0;
		constexpr i32 efRadiationalBlur_var_ptr = 0x0d75a8;
		constexpr i32 efFlash_func_proc = 0x04e560;
		constexpr i32 efFlash_var_ptr = 0x1a6b7c;
		constexpr i32 efFlash_func_proc_ptr = 0x0a5a28;
		constexpr i32 efDirectionalBlur_func_proc_ptr = 0x0a00a0;
		constexpr i32 efDirectionalBlur_var_ptr = 0x0d75cc;
		constexpr i32 efDirectionalBlur_Filter_mt_func_call = 0x00cae6;
		constexpr i32 efLensBlur_Media_mt_func_call = 0x012761;
		constexpr i32 efLensBlur_Filter_mt_func_call = 0x012786;
		constexpr i32 efLensBlur_var_ptr = 0x11ec5c;
		constexpr i32 efMotionBlur_var_ptr = 0x1bad34;
		constexpr i32 efChromakey_var_ptr = 0x11ec7c;

		constexpr i32 efPortionFilter = 0x0a92c0;
		constexpr i32 efWaveform = 0x0ba300;

		constexpr i32 PixelYCA_ssss2fbbs = 0x070220;
		constexpr i32 PixelYCA_fbbs2ssss = 0x0703f0;
		constexpr i32 PixelYCA_sss2fbb = 0x070550;
		constexpr i32 PixelYCA_fbb2sss = 0x070700;

		constexpr i32 ScriptProcessingFilter = 0x1b2b10;

		constexpr i32 extract_extension = 0x04e1d0;
		constexpr i32 ini_extension_buf = 0x14cb58;
		constexpr i32 str_DOUGAFILE= 0x09df6c; // "動画ファイル"
		constexpr i32 str_ONSEIFILE = 0x0ba698; // "音声ファイル"
		constexpr i32 str_dot_exedit_backup = 0x0a5308;
		constexpr i32 str_dot_aup = 0x0a5320;
		constexpr i32 str_file = 0x09d7d0;
		constexpr i32 str_type = 0x09d6c4;
		constexpr i32 str_name = 0x09d6b4;
		constexpr i32 str_mode = 0x09e04c;
		constexpr i32 str_calc = 0x0a2818;
		constexpr i32 str_range = 0x0a3db4;
		constexpr i32 str_color = 0x09d7c0;
		constexpr i32 str_no_color = 0x0a5c80;
		constexpr i32 str_color_yc = 0x0a0fb0;
		constexpr i32 str_status = 0x0a0fa8;
		constexpr i32 str_blend = 0x0a30e8;
		constexpr i32 str_RGB_d_d_d = 0x0a2d24;
		constexpr i32 str_SHITEINASHI_MOTOGAZOUNOIRO = 0x0a5c8c;


		constexpr i32 GetCache = 0x00cff0;
		constexpr i32 CreateCache = 0x00cd00;
		constexpr i32 GetOrCreateCache = 0x04d7d0;
		constexpr i32 CacheInfo = 0x0d7710;
		constexpr i32 CachePriorityCount = 0x11eb1c;

		constexpr i32 video_func_main = 0x048830;
		constexpr i32 video_func_idx = 0x1a5384;

		constexpr i32 GetOrCreateSceneBufYC = 0x02a770;
		constexpr i32 GetOrCreateSceneBufYCA = 0x02a830;
		constexpr i32 SceneDisplaying = 0x1a5310;
		constexpr i32 SceneSetting = 0x177a50;
		constexpr i32 get_scene_image = 0x04ce20;
		constexpr i32 get_scene_size = 0x02b980;
		constexpr i32 scene_has_alpha = 0x02ba00;

		constexpr i32 set_undo = 0x08d290;

		constexpr i32 frame_cursor = 0x1a5304;
		constexpr i32 frame_n = 0x14d3a0;
		constexpr i32 double_framerate = 0x0a4068;
		constexpr i32 double_framerate_scale = 0x0a4060;

		constexpr i32 edit_open = 0x03ac30;
		constexpr i32 update_any_exdata = 0x04a7e0;
		constexpr i32 get_same_filter_idx_if_leader = 0x0365b0;

		constexpr i32 do_multi_thread_func = 0x06c650;
		constexpr i32 dlg_get_load_name = 0x020900;

		constexpr i32 exfunc = 0x0a41e0;
		constexpr i32 exfunc_08 = 0x04ab40;
		constexpr i32 exfunc_10 = 0x04abe0;
		constexpr i32 exfunc_1c = 0x04ade0;
		constexpr i32 exfunc_44 = 0x081b40;
		constexpr i32 exfunc_4c = 0x04a430;
		constexpr i32 exfunc_64 = 0x04d040;
		constexpr i32 exfunc_6c = 0x04d2a0;
		constexpr i32 func_0x047ad0 = 0x047ad0;
		constexpr i32 scenechange_progress_times4096 = 0x230c60;
		constexpr i32 GetCurrentProcessing = 0x047ba0;
		constexpr i32 LoadedFilterTable = 0x187c98;
		constexpr i32 splitted_object_new_group_belong = 0x034f90;
		constexpr i32 DrawTimelineMask = 0x0392f0;
		constexpr i32 InitScrollHorizonal = 0x038d30;
		constexpr i32 disp_settingdialog = 0x039550;
		constexpr i32 filter_sendmessage = 0x04a1a0;
		constexpr i32 get_near_object_idx = 0x0445a0;
		constexpr i32 TraScript_ProcessingObjectIndex = 0x1b2b04;
		constexpr i32 TraScript_ProcessingTrackBarIndex = 0x1b21f4;
		constexpr i32 TraScript_Time = 0x1b28c8;

		constexpr i32 LoadLua = 0x0646e0;
		constexpr i32 hmodule_lua = 0x1bac9c;
		constexpr i32 luaL_Reg_global_table = 0x09a680;
		constexpr i32 luaL_Reg_obj_table = 0x09a5c0;
		constexpr i32 exeditdir = 0x1b2b18;
		constexpr i32 sScriptFolderName = 0x1b2b4c;

		constexpr i32 effect_proc = 0x049370;
		constexpr i32 obj_effect_noarg = 0x04b200;

		constexpr i32 l_effect = 0x05d0a0;
		constexpr i32 l_load = 0x05ef50;
		constexpr i32 l_draw = 0x05e250;
		constexpr i32 l_drawpoly = 0x05e680;
		constexpr i32 l_setanchor = 0x0625e0;

		constexpr i32 GetOrCreateLuaState = 0x064660;
		constexpr i32 LuaUnload = 0x064e90;
		constexpr i32 SetLuaPathAndCpath = 0x064550;
		constexpr i32 DoScriptInit = 0x0641a0;
		constexpr i32 DoScriptExit = 0x064250;
		constexpr i32 luastateidx = 0x1baccc;

		constexpr i32 lua_pop_nop = 0x064d15;
		constexpr i32 lua_set_nop = 0x06442a;
		constexpr i32 lua_tonumber_arg = 0x064d64;
		constexpr i32 lua_tostring_calling1 = 0x064d44;
		constexpr i32 lua_tostring_calling2 = 0x06448b;
		constexpr i32 lua_tostring_calling3 = 0x064449;

		constexpr i32 OutputDebugString_calling_err1 = 0x064453;
		constexpr i32 OutputDebugString_calling_err2 = 0x064495;
		constexpr i32 OutputDebugString_calling_err3 = 0x064d4e;
		constexpr i32 OutputDebugString_calling_dbg = 0x05d099;

		constexpr i32 ignore_media_param_reset_mov = 0x00674d;
		constexpr i32 ignore_media_param_reset_aud = 0x090116;

		constexpr i32 ExtendedFilter_wndcls = 0x02e872;

		constexpr i32 loaded_spi_array = 0x2321f0;

		constexpr i32 MyFindFirstFile = 0x04e220;
		constexpr i32 MyFindNextFile = 0x04e270;
		constexpr i32 LoadSpi = 0x08a210;

		constexpr i32 text_op_logfont_size = 0x050716;
		constexpr i32 specialcolorconv_status2 = 0x0a14f4;
		constexpr i32 current_font_name = 0x236368;
		constexpr i32 current_font_height = 0x23634c;

		constexpr i32 ObjectAllocNum = 0x1e0fa0;
		constexpr i32 ObjectArrayPointer = 0x1e0fa4;
		constexpr i32 ExdataPointer = 0x1e0fa8;
		constexpr i32 NextObjectIdxArray = 0x1592d8;

		constexpr i32 get_exdata_ptr = 0x047b40;

		constexpr i32 SelectingObjectNum = 0x167d88;
		constexpr i32 SelectingObjectIdxArray = 0x179230;
		constexpr i32 deselect_object = 0x34eb0;

		constexpr i32 object2idx = 0x02b0d0;

		constexpr i32 SettingDialog_ObjIdx = 0x177a10;
		constexpr i32 get_last_filter_idx = 0x0335f0;
		constexpr i32 get_filterp = 0x047b00;
		constexpr i32 delete_filter_effect = 0x033d20;

		constexpr i32 TrackModeInfoArray = 0x14d3b0;
		constexpr i32 TrackLeftInfoArray = 0x14d4c8;
		constexpr i32 TrackRightInfoArray = 0x14def0;
		constexpr i32 TrackParamInfoArray = 0x14e900;
		constexpr i32 update_track = 0x2c470;

		constexpr i32 ScaleColorBackGround = 0x0a4048;
		constexpr i32 ScaleColorForeGround = 0x0a404c;
		constexpr i32 LayerNameRectWidth = 0x037d1a;
		constexpr i32 layer_height_array = 0x0a3e08;
		
		//constexpr i32 LayerLockBorder = 0x037d78; // push char されているので安直には無理
		constexpr i32 LayerLockBorder_mod = 0x037d71;
		constexpr i32 LayerLockCenter = 0x037d81;
		constexpr i32 LayerClippingBorder = 0x037e96;
		constexpr i32 LayerClippingCenter = 0x037ea2;
		constexpr i32 LayerLinkBorder = 0x037dfd;
		constexpr i32 LayerLinkCenter = 0x037e09;
		constexpr i32 LayerHideAlpha = 0x09a570;

		constexpr i32 ObjectColorMedia = 0x0a40e0;
		constexpr i32 ObjectColorMFilter = 0x0a4104;
		constexpr i32 ObjectColorSound = 0x0a4128;
		constexpr i32 ObjectColorSFilter = 0x0a414c;
		constexpr i32 ObjectColorControl = 0x0a4170;
		constexpr i32 ObjectColorUnknown = 0x0a4194;
		constexpr i32 ObjectColorInactive = 0x0a41b8;

		constexpr i32 ObjectNameColor = 0x0a4040;
		constexpr i32 ObjectNameColorShow = 0x0a4040;
		constexpr i32 ObjectNameColorHide = 0x0a4044;
		constexpr i32 MidPointSize = 0x0a3e14;
		constexpr i32 ObjectClippingColorB = 0x0375d7;
		constexpr i32 ObjectClippingColorG = 0x0375d9;
		constexpr i32 ObjectClippingColorR = 0x0375db;
		constexpr i32 ObjectClippingHeight = 0x0375e2;

		constexpr i32 BPMGridColorBeat = 0x0a4050;
		constexpr i32 BPMGridColorMeasure = 0x0a4054;

	}
}
