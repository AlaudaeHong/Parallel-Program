if __name__ == '__main__':
	import timeit
	import matplotlib.pyplot as plt
	import numpy as np
	import re

	y1 = [0.065659, 0.032880, 0.016572, 0.008467, 0.004860, 0.004263, 0.004258, 0.004049]
	y2 = [0.066313, 0.033183, 0.016640, 0.008796, 0.004730, 0.003057, 0.003061, 0.003386]
	
	x = [64, 128, 256, 512, 1024, 2048, 4096, 8192]

    

	plt.plot(x, y1, label="MPI_p2p_Reduce")
	plt.plot(x, y2, label="MPI_Reduce")
	plt.xscale('log', basex = 2)
	plt.ylabel('Execute time (second)', fontsize=16)
	plt.xlabel('# rank', fontsize=16)
	plt.legend()
	plt.show()


