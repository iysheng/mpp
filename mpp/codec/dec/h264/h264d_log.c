﻿/*
*
* Copyright 2015 Rockchip Electronics Co. LTD
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "mpp_env.h"
#include "mpp_mem.h"
#include "h264d_log.h"

#define MODULE_TAG   "h264d_log"

#define LOG_BUF_SIZE 512

#define GetBitVal(val, pos)   ( ( (val)>>(pos) ) & 0x1 & (val) )

const LogEnvStr_t logenv_name = {
	"h264d_log_help",
	"h264d_log_show",
	"h264d_log_ctrl",
	"h264d_log_level",
	"h264d_log_outpath",
	"h264d_log_decframe",
	"h264d_log_begframe",
	"h264d_log_endframe",
};

const char *loglevel_name[LOG_LEVEL_MAX] = {
	"SILENT  ",
	"FATAL   ",
	"ERROR   ",
	"WARNNING",
	"INFO    ",	
	"TRACE   ",	
};

const char *logctrl_name[LOG_MAX] = {
	"DEBUG_EN      ",
	"FPGA_MODE     ",
	"LOG_PRINT     ",
	"LOG_WRITE     ",
	"READ_NALU     ",  
	"READ_SPS      ",  
	"READ_SUBSPS   ",  
	"READ_PPS      ",  
	"READ_SLICE    ",  
	"WRITE_SPSPPS  ",  
	"WRITE_RPS     ",  
	"WRITE_SCANLIST",  
	"WRITE_STEAM   ",  
	"WRITE_REG     ",
};

const char *log_array_name[LOG_MAX] = {
	"NULL          ",
	"FPGA_DATA     ",
	"PAESE_RUN     ",
	"HAL_RUN       ",
	"READ_NALU     ",  
	"READ_SPS      ",  
	"READ_SUBSPS   ",  
	"READ_PPS      ",  
	"READ_SLICE    ",  
	"WRITE_SPSPPS  ",  
	"WRITE_RPS     ",  
	"WRITE_SCANLIST",  
	"WRITE_STEAM   ",  
	"WRITE_REG     ",
};


static MPP_RET get_logenv(LogEnv_t *env)
{
	//!< read env
	mpp_env_get_u32(logenv_name.help,     &env->help,     0);	
	mpp_env_get_u32(logenv_name.show,     &env->show,     0);
	mpp_env_get_u32(logenv_name.ctrl,     &env->ctrl,     0);
	mpp_env_get_u32(logenv_name.level,    &env->level,    0);	
	mpp_env_get_u32(logenv_name.decframe, &env->decframe, 0);
	mpp_env_get_u32(logenv_name.begframe, &env->begframe, 0);
	mpp_env_get_u32(logenv_name.endframe, &env->endframe, 0);
	mpp_env_get_str(logenv_name.outpath,  &env->outpath,  NULL);

	return MPP_OK;
}


static void print_env_help(LogEnv_t *env)
{
	RK_U8 i = 0;

	fprintf(stdout, "--------------- h264d help  -------------------- \n");

	fprintf(stdout, "h264d_log_help     :[num] (0/1) show help content \n");
	fprintf(stdout, "h264d_log_show     :[num] (0/1) show current log setting \n");
	fprintf(stdout, "h264d_log_outpath  :[str]  \n");
	fprintf(stdout, "h264d_log_decframe :[num]  \n");
	fprintf(stdout, "h264d_log_begframe :[num]  \n");
	fprintf(stdout, "h264d_log_endframe :[num]  \n");
	fprintf(stdout, "h264d_log_level    :[num]  \n");
	for (i = 0; i < LOG_LEVEL_MAX; i++)
	{
		fprintf(stdout, "                    (%2d) -- %s  \n", i, loglevel_name[i]);
	}
	fprintf(stdout, "\nh264d_log_ctrl     :[32bit] \n");
	for (i = 0; i < LOG_MAX; i++)
	{
		fprintf(stdout, "                    (%2d)bit -- %s  \n", i, logctrl_name[i]);
	}
	fprintf(stdout, "------------------------------------------------- \n");
}

static void show_env_flags(LogEnv_t *env)
{
	RK_U8 i = 0;
	fprintf(stdout, "------------- h264d debug setting   ------------- \n");
	fprintf(stdout, "outputpath    : %s \n", env->outpath);
	fprintf(stdout, "DecodeFrame   : %d \n", env->decframe);
	fprintf(stdout, "LogBeginFrame : %d \n", env->begframe);
	fprintf(stdout, "LogEndFrame   : %d \n", env->endframe);
	fprintf(stdout, "LogLevel      : %s \n", loglevel_name[env->level]);
	for (i = 0; i < LOG_MAX; i++)
	{
		fprintf(stdout, "%s: %d (%d)\n", logctrl_name[i], GetBitVal(env->ctrl, i), i);
	}
	fprintf(stdout, "------------------------------------------------- \n");
}

static MPP_RET explain_ctrl_flag(RK_U32 ctrl_val, LogFlag_t *pflag)
{
	pflag->print_en = GetBitVal(ctrl_val, LOG_PRINT         );
	pflag->write_en = GetBitVal(ctrl_val, LOG_WRITE         );
	pflag->debug_en = GetBitVal(ctrl_val, LOG_DEBUG_EN      )
		           && GetBitVal(ctrl_val, LOG_READ_NALU     )
		           && GetBitVal(ctrl_val, LOG_READ_SPS      )
		           && GetBitVal(ctrl_val, LOG_READ_SUBSPS   )
		           && GetBitVal(ctrl_val, LOG_READ_PPS      )
		           && GetBitVal(ctrl_val, LOG_READ_SLICE    )
		           && GetBitVal(ctrl_val, LOG_WRITE_SPSPPS  )
		           && GetBitVal(ctrl_val, LOG_WRITE_RPS     )
		           && GetBitVal(ctrl_val, LOG_WRITE_SCANLIST)
		           && GetBitVal(ctrl_val, LOG_WRITE_STEAM   )
		           && GetBitVal(ctrl_val, LOG_WRITE_REG     );

	return MPP_OK;
}

static void close_log_files(LogEnv_t *env)
{
	FCLOSE(env->fp_driver);
	FCLOSE(env->fp_syn_parse);
	FCLOSE(env->fp_syn_hal);
	FCLOSE(env->fp_run_parse);
	FCLOSE(env->fp_run_hal);
}
static MPP_RET open_log_files(LogEnv_t *env, LogFlag_t *pflag)
{
	char fname[128] = { 0 };

	RET_CHECK(!pflag->write_en);

	//!< runlog file
	if (GetBitVal(env->ctrl, LOG_DEBUG_EN))
	{
		sprintf(fname, "%s/h264d_parse_runlog.dat", env->outpath);
		FLE_CHECK(env->fp_run_parse = fopen(fname, "wb"));
		sprintf(fname, "%s/h264d_hal_runlog.dat", env->outpath);
		FLE_CHECK(env->fp_run_hal = fopen(fname, "wb"));
	}
	//!< fpga drive file
	if (GetBitVal(env->ctrl, LOG_FPGA))
	{
		sprintf(fname, "%s/h264d_driver_data.dat", env->outpath);
		FLE_CHECK(env->fp_driver = fopen(fname, "wb"));
	}
	//!< read syntax
	if (   GetBitVal(env->ctrl, LOG_READ_NALU  )
		|| GetBitVal(env->ctrl, LOG_READ_SPS   )
		|| GetBitVal(env->ctrl, LOG_READ_SUBSPS)
		|| GetBitVal(env->ctrl, LOG_READ_PPS   )
		|| GetBitVal(env->ctrl, LOG_READ_SLICE ) )
	{
		sprintf(fname, "%s/h264d_read_syntax.dat", env->outpath);
		FLE_CHECK(env->fp_syn_parse = fopen(fname, "wb"));
	}
	//!< write syntax
	if (   GetBitVal(env->ctrl, LOG_WRITE_SPSPPS  )
		|| GetBitVal(env->ctrl, LOG_WRITE_RPS     )
		|| GetBitVal(env->ctrl, LOG_WRITE_SCANLIST)
		|| GetBitVal(env->ctrl, LOG_WRITE_STEAM   )
		|| GetBitVal(env->ctrl, LOG_WRITE_REG     ) )
	{
		sprintf(fname, "%s/h264d_write_syntax.dat", env->outpath);
		FLE_CHECK(env->fp_syn_hal = fopen(fname, "wb"));
	}

__RETURN:
	return MPP_OK;

__FAILED:
	return MPP_NOK;
}

MPP_RET h264d_log_deinit(H264dLogCtx_t *logctx)
{
	mpp_free(logctx->buf);
	close_log_files(&logctx->env);

	return MPP_OK;
}

MPP_RET h264d_log_init(H264dLogCtx_t *logctx)
{
	RK_U8 i = 0;
	MPP_RET ret = MPP_NOK;
	LogCtx_t *pcur = NULL;

	FUN_CHECK(ret = get_logenv(&logctx->env));
	if (logctx->env.help)
	{
		print_env_help(&logctx->env);
	}
	if (logctx->env.show)
	{
		show_env_flags(&logctx->env);
	}
	FUN_CHECK(ret = explain_ctrl_flag(logctx->env.ctrl, &logctx->log_flag));
	if ( !logctx->log_flag.debug_en
		&& !logctx->log_flag.print_en && !logctx->log_flag.write_en )
	{
		logctx->log_flag.debug_en = 0;
		goto __RETURN;
	}
	logctx->log_flag.level = (1 << logctx->env.level) - 1;
	//!< open file
	FUN_CHECK(ret = open_log_files(&logctx->env, &logctx->log_flag));
	//!< set logctx
	MEM_CHECK(logctx->buf = mpp_calloc(LogCtx_t, LOG_MAX));

	while(i < LOG_MAX)
	{
		if (GetBitVal(logctx->env.ctrl, i))
		{
			pcur = logctx->parr[i] = &logctx->buf[i];
			pcur->tag = log_array_name[i];
			pcur->flag = &logctx->log_flag;

			switch(i)
			{			
			case LOG_FPGA:
				pcur->fp = logctx->env.fp_driver;
				break;
			case RUN_PARSE:
				pcur->fp = logctx->env.fp_run_parse;
				break;
			case RUN_HAL:
				pcur->fp = logctx->env.fp_run_hal;
				break;
			case LOG_READ_NALU:  
			case LOG_READ_SPS:  
			case LOG_READ_SUBSPS:  
			case LOG_READ_PPS:  
			case LOG_READ_SLICE:  
				pcur->fp = logctx->env.fp_syn_parse;
				break;
			case LOG_WRITE_SPSPPS:  
			case LOG_WRITE_RPS:
			case LOG_WRITE_SCANLIST:  
			case LOG_WRITE_STEAM:  
			case LOG_WRITE_REG:
				pcur->fp = logctx->env.fp_syn_hal;
			default:
				break;
			}
		}
		i++;
	}

__RETURN:
	return ret = MPP_OK;
__FAILED:
	logctx->log_flag.debug_en = 0;
	h264d_log_deinit(logctx);

	return ret;
}


/*!
***********************************************************************
* \brief
*   write log function
***********************************************************************
*/
void writelog(LogCtx_t *ctx, char *filename, RK_U32 line, char *loglevel, const char *msg, ...)
{
#if __DEBUG_EN

	va_list argptr;
	char argbuf[LOG_BUF_SIZE] = { 0 };
	char *pfn = NULL, *pfn0 = NULL, *pfn1 = NULL;

	va_start(argptr, msg);
	vsnprintf(argbuf, sizeof(argbuf), msg, argptr);

	pfn0 = strrchr(filename, '/');
	pfn1 = strrchr(filename, '\\');
	pfn  = pfn0 ? (pfn0 + 1) : (pfn1 ? (pfn1 + 1) : filename);

	if (ctx->flag->print_en)
	{
		printf("[ TAG = %s ], file: %s, line: %d, [ %s ], %s \n", ctx->tag, pfn, line, loglevel, argbuf);
	}
	if (ctx->fp && ctx->flag->write_en)
	{
		fprintf(ctx->fp, "[ TAG = %s ], file: %s, line: %d, [ %s ], %s \n", ctx->tag, pfn, line, loglevel, argbuf);
		fflush(ctx->fp);
	}
	va_end(argptr);

#endif
}
