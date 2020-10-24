local io = require("io")
local string = require("string")


-- 写入的头文件
local h_name = './mnp_h264_res.h'


-- 需要导入的lua文件
local files = {
	{'./2019.432x240.X264.AAC.Mandarin.240.10.h264',	'g_mnp_h264_test1'},
	{'./2019.432x240.X264.AAC.Mandarin.240.10.flv',		'g_mnp_flv_test2'}
}


local fout = io.open(h_name, 'w')
assert(fout)


-- 文件头
fout:write('#ifndef __MNP_H264_RES_H__\n')
fout:write('#define __MNP_H264_RES_H__\n')
fout:write('#include <stdio.h>\n')
fout:write('#include <string.h>\n')
fout:write('\n')


-- 结构体定义
local def_st_map = '#define MNP_MAP_BUF   68\
typedef struct mnp_map_t_\
{\
    char    name[MNP_MAP_BUF];\
    char*   p_data;\
    int     data_len;\
}mnp_map_t;\n'

fout:write(def_st_map)
fout:write('\n')


local write_array = function (pf, r_name, k_name)
	local BUFSIZE = 1024 * 1024

	local key_name = k_name
	local name = r_name
	local path = name

	local fin = io.open(path, 'r')
	assert(fin)

	pf:write('const unsigned char ')
	pf:write(key_name)
	pf:write('[] = {')

	while true do
		local lines, rest = fin:read(BUFSIZE, '*all')
		
		if not lines then break end

		if rest then lines = lines .. rest .. '\n' end
		
		local len = string.len(lines)
	
		for i = 1, len do
			pf:write(string.byte(lines, i));
			
			pf:write(',');
		end
	end

	pf:write('0};\n')
end


-- 写文件二进制数组
for i = 1, #files do
	write_array(fout, files[i][1], files[i][2])
end

-- 写map信息
fout:write('\n')
fout:write('mnp_map_t g_res_key_map[] = {\n')

for i = 1, #files do
	local tmp_str = string.format("    {\"%s\", (char*)%s, sizeof(%s) - 1},\n", files[i][2], files[i][2], files[i][2])
	fout:write(tmp_str)
end

fout:write('    {"", NULL, 0}\n')
fout:write('};\n')


-- 文件末尾
fout:write('\n')
fout:write('#endif\n')
fout:close()
