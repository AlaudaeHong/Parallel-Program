if __name__ == '__main__':
    import timeit
    import matplotlib.pyplot as plt
    import numpy as np
    import re

    nob = [0, 0, 0, 0, 0, 0]
    b = [0, 0, 0, 0, 0, 0]

    rank = [ 2, 4, 8, 16, 32]


    for i in range(0, 5000):
        #f = open("C:/Users/Acer-PC/OneDrive/Parallel Programming/hw2/r2", "r");
        f = open("r2", "r");
        line = [float(x.group()) for x in re.finditer(r'\d+\.\d+', f.readline())]

        for j in range(0, 6):
            nob[j] += line[2*j]
            b[j] += line[2*j + 1]

        line.clear()

    temp1 = [0,0,0,0,0]
    temp2 = [0,0,0,0,0]

    for i in range(1, 6):
        nob[i] = float(nob[0] - nob[i]) / nob[0]
        #print(nob[i])
        nob[i] *= 100
        temp1[i-1] = nob[i]
        b[i] = float(b[0] - b[i]) / b[0]
        b[i] *= 100
        temp2[i-1] = b[i]

    

    plt.plot(rank, temp1, label="No MPI_Barrier")
    plt.plot(rank, temp2, label="MPI_Barrier")
    plt.xscale('log', basex = 2)
    plt.legend()
    plt.show()


