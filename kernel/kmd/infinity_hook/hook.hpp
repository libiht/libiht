#pragma once
#include "headers.hpp"

namespace k_hook
{
	// SSDT回调函数
	typedef void(__fastcall* f_call_back)(unsigned int new_proc, unsigned int old_proc);

	// 初始化数据
	bool initialize(f_call_back ssdt_call_back);

	// 开始拦截函数调用
	bool start();

	// 结束拦截函数调用
	bool stop();
}