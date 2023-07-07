#include <stdio.h>
#include <b3ddepth/utils/TLog.h>

#define AE_EXP_GAIN_PARAM_FILE_NAME2 "ae2.file"
#define AWB_GAIN_PARAM_FILE_NAME2 "awb2.file"

#define AE_EXP_GAIN_PARAM_FILE_NAME8 "ae8.file"
#define AWB_GAIN_PARAM_FILE_NAME8 "awb8.file"

typedef unsigned short cmr_u16;
typedef unsigned int cmr_u32;

typedef signed short cmr_s16;
typedef signed int cmr_s32;

struct ae_exposure_param {
    cmr_u32 cur_index;
    cmr_u32 line_time;
    cmr_u32 exp_line;
    cmr_u32 exp_time;
    cmr_s32 dummy;
    cmr_s32 frm_len;
    cmr_s32 frm_len_def;
    cmr_u32 gain;			/*gain = sensor_gain * isp_gain */
    cmr_u32 sensor_gain;
    cmr_u32 isp_gain;
    cmr_s32 target_offset;
    cmr_s32 bv;
    cmr_u32 table_idx;
};

struct ae_exposure_param_switch {
    cmr_u32 target_offset;
    cmr_u32 exp_line;
    cmr_u32 exp_time;
    cmr_s32 dummy;
    cmr_u32 frm_len;
    cmr_u32 frm_len_def;
    cmr_u32 gain;
    cmr_u32 table_idx;
};

struct awb_save_gain {
    cmr_u16 r;
    cmr_u16 g;
    cmr_u16 b;
    cmr_u16 ct;
};

using namespace b3dd;

void ae_read_exp_gain_param(struct ae_exposure_param *param, cmr_u32 num, struct ae_exposure_param_switch * ae_manual_param)
{
    cmr_u32 i = 0;
    FILE *pf = NULL, *pf2 = NULL;
    pf = fopen(AE_EXP_GAIN_PARAM_FILE_NAME2, "rb");
    pf2 = fopen(AE_EXP_GAIN_PARAM_FILE_NAME8, "rb");
    if (pf) {
        memset((void *)param, 0, sizeof(struct ae_exposure_param) * num);
        fread((char *)param, 1, num * sizeof(struct ae_exposure_param), pf);
        fread((char *)ae_manual_param, 1, num * sizeof(struct ae_exposure_param_switch), pf);
        fclose(pf);
        pf = NULL;

        logInfo("========== 2M ==========");
        logInfo("line_time %d", param[0].line_time);
        logInfo("exp_time %d", param[0].exp_time);
        logInfo("exp_line %d", param[0].exp_line);
        logInfo("dummy %d", param[0].dummy);
        logInfo("frm_len %d", param[0].frm_len);
        logInfo("frm_len_def %d", param[0].frm_len_def);
        logInfo("gain %d", param[0].gain);
        logInfo("sensor_gain %d", param[0].sensor_gain);
        logInfo("isp_gain %d", param[0].isp_gain);
        logInfo("target_offset %d", param[0].target_offset);
        logInfo("bv %d", param[0].bv);
        logInfo("table_idx %d", param[0].table_idx);


    }

    if (pf2) {
        memset((void *)param, 0, sizeof(struct ae_exposure_param) * num);
        fread((char *)param, 1, num * sizeof(struct ae_exposure_param), pf2);
        fread((char *)ae_manual_param, 1, num * sizeof(struct ae_exposure_param_switch), pf2);
        fclose(pf2);
        pf2 = NULL;

        logInfo("========== 8M ==========");
        logInfo("line_time %d", param[0].line_time);
        logInfo("exp_time %d", param[0].exp_time);
        logInfo("exp_line %d", param[0].exp_line);
        logInfo("dummy %d", param[0].dummy);
        logInfo("frm_len %d", param[0].frm_len);
        logInfo("frm_len_def %d", param[0].frm_len_def);
        logInfo("gain %d", param[0].gain);
        logInfo("sensor_gain %d", param[0].sensor_gain);
        logInfo("isp_gain %d", param[0].isp_gain);
        logInfo("target_offset %d", param[0].target_offset);
        logInfo("bv %d", param[0].bv);
        logInfo("table_idx %d", param[0].table_idx);


    }

}

void awb_read_gain(struct awb_save_gain *cxt, cmr_u32 num)
{
    cmr_u32 i = 0;
    FILE *fp = NULL, *fp2 = NULL;
    fp = fopen(AWB_GAIN_PARAM_FILE_NAME2, "rb");
    fp2 = fopen(AWB_GAIN_PARAM_FILE_NAME8, "rb");
    if (fp) {
        memset((void *)cxt, 0, sizeof(struct awb_save_gain) * num);
        fread((char *)cxt, 1, num * sizeof(struct awb_save_gain), fp);
        fclose(fp);
        fp = NULL;

        logInfo("========== 2M ==========");
        logInfo("r %d", cxt[0].r);
        logInfo("g %d", cxt[0].g);
        logInfo("b %d", cxt[0].b);
        logInfo("ct %d", cxt[0].ct);
        for (i = 0; i < num; ++i) {
            logInfo("[%d]: %d, %d, %d, %d\n", i, cxt[i].r, cxt[i].g, cxt[i].b, cxt[i].ct);
        }
    }

    if (fp2) {
        memset((void *)cxt, 0, sizeof(struct awb_save_gain) * num);
        fread((char *)cxt, 1, num * sizeof(struct awb_save_gain), fp2);
        fclose(fp2);
        fp2 = NULL;

        logInfo("========== 8M ==========");
        logInfo("r %d", cxt[0].r);
        logInfo("g %d", cxt[0].g);
        logInfo("b %d", cxt[0].b);
        logInfo("ct %d", cxt[0].ct);
        for (i = 0; i < num; ++i) {
            logInfo("[%d]: %d, %d, %d, %d\n", i, cxt[i].r, cxt[i].g, cxt[i].b, cxt[i].ct);
        }
    }
}



void main() {
    b3dd::TLog::setLogLevel(TLog::LOG_INFO);
    static struct ae_exposure_param s_bakup_exp_param[4] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
    static struct ae_exposure_param_switch s_ae_manual[4] = { { 0,0,0,0,0,0,0,0 },{ 0,0,0,0,0,0,0,0 },{ 0,0,0,0,0,0,0,0 },{ 0,0,0,0,0,0,0,0 } };

    struct awb_save_gain s_save_awb_param[4] = { { 0, 0, 0, 0 },{ 0, 0, 0, 0 },{ 0, 0, 0, 0 },{ 0, 0, 0, 0 } };
    ae_read_exp_gain_param(&s_bakup_exp_param[0], sizeof(s_bakup_exp_param) / sizeof(struct ae_exposure_param), &s_ae_manual[0]);
    awb_read_gain(&s_save_awb_param[0], sizeof(s_save_awb_param) / sizeof(struct awb_save_gain));
}