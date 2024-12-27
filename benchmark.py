#!/usr/bin/python3
import os
import shutil
#bm = ["bwaves_s","gcc_s"]
#assoc:8
size = ["512kB","1MB","2MB"]
#size = ["64kB","128kB","256kB","512kB","1MB","2MB"]
#assoc:16
#size = ["512kB"]
#bm = ["perlbench_s","gcc_s","mcf_s"] ## results_fast_1 sc = 0.25
#bm = ["xalancbmk_s","exchange2_s","specrand_is","lbm_s","deepsjeng_s","xz_s"] ## results_fast_2 sc=0.25
#bm = ["nab_s","fotonik3d_s","specrand_fs"] ## results_fast_3 sc = 0.25
#bm = ["cactuBSSN_s","cam4_s","imagick_s"] ##results_fast_4 sc = 0.25
#bm = ["xalancbmk_s","exchange2_s","specrand_is","lbm_s","deepsjeng_s","xz_s"] ## results_fast_5 sc=0.5 --> long int
#bm = ["fotonik3d_s","specrand_fs","perlbench_s","gcc_s","mcf_s"] ## results_fast_6 sc=0.5 --> long int
bm = ["cactuBSSN_s","cam4_s","imagick_s","nab_s"] ## results_fast_7 sc=0.5 --> long int
#bm = ["bwaves_s","cactuBSSN_s","cam4_s","imagick_s","nab_s","fotonik3d_s","specrand_fs","perlbench_s","gcc_s","mcf_s","xalancbmk_s","exchange2_s","specrand_is","lbm_s","deepsjeng_s","xz_s"]

for j in size:
	out = ""
	out_cpi = ""
	out_miss = ""
	for i in bm:
		#cmd = "./build/ECE565-X86/gem5.opt configs/spec/spec_se.py -b {}  --maxinsts=50000000 --cpu-type=TimingSimpleCPU --caches --l2cache  --l2_size={} --l2_assoc=16 --cacheline_size=64".format(i,j)
		cmd = "./build/ECE565-X86/gem5.fast configs/spec/spec_se.py -b {}  --maxinsts=500000000 --cpu-type=TimingSimpleCPU --caches --l2cache --l1d_size=16kB --l1i_size=16kB --l2_size={} --l2_assoc=16 --cacheline_size=64".format(i,j)
		os.system(cmd)
		origin = "m5out/stats.txt"
		t_f = open(origin,'r')
		lines = t_f.readlines()
		for line in lines:
			if "system.l2.overallMissRate::total" in line:
				line.strip()
				tokens = line.split(" ")
				tokens = [k for k in tokens if k]
				out = out + i + ","  + tokens[1] + "\n" 
			if "system.cpu.numCycles" in line:
				line.strip()
				tokens = line.split(" ")
				tokens = [k for k in tokens if k]
				out_cpi = out_cpi + i + ","  + tokens[1] + "\n" 
			if "system.l2.overallMisses::total" in line:
				line.strip()
				tokens = line.split(" ")
				tokens = [k for k in tokens if k]
				out_miss = out_miss + i + ","  + tokens[1] + "\n" 
		t_f.close()
		f = i + "_"+ j + "_"+"stats.txt"
		target = "results_fast_7/" + f
		cp_f = "touch " + target
		os.system(cp_f)
		shutil.copy(origin, target)

	out_f = "results_fast_7/" + j + "_out.txt"
	out_fh = open(out_f,'w')
	out_fh.write(out)
	out_fh.close()
	out_f_cpi = "results_fast_7/" + j + "_cpi_out.txt"
	out_fh_cpi = open(out_f_cpi,'w')
	out_fh_cpi.write(out_cpi)
	out_fh_cpi.close()
	out_f_miss = "results_fast_7/" + j + "_miss_out.txt"
	out_fh_miss = open(out_f_miss,'w')
	out_fh_miss.write(out_miss)
	out_fh_miss.close()
