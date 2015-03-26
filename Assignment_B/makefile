CC=gcc-4.9
NVCC=nvcc

NVCC_FLAGS_GPU=-O3 -arch sm_20 -Xcompiler -fopenmp -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_BACKEND_CUDA -lgomp -D_GPU_
NVCC_FLAGS_CPU=-O3 -Xcompiler -fopenmp -DTHRUST_DEVICE_SYSTEM=THRUST_DEVICE_BACKEND_OMP -lgomp -D_CPU_

CFLAGS=-O3 -Wall
LIB=-L/share/apps/boost/1.56.0/lib -lboost_serialization
INCLUDE=-I/share/apps/boost/1.56.0/include
LIB=-lboost_serialization -lnvidia-ml 
#-L/Users/weiyi/Desktop/BDiF2015-Weiyi-Chen/Assignment_B/lib -lnvidia-ml
#INCLUDE=-I/Users/weiyi/Desktop/BDiF2015-Weiyi-Chen/Assignment_B/include



EXE=CVA
SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)
CU_SRC=$(wildcard *.cu)
CU_OBJ_CPU=$(CU_SRC:.cu=_cpu.o)
CU_OBJ_GPU=$(CU_SRC:.cu=_gpu.o)

all:gpu
cpu:$(EXE)_cpu.x
gpu:$(EXE)_gpu.x

$(EXE)_cpu.x:$(OBJ) $(CU_OBJ_CPU)
	$(NVCC) $(OBJ) $(CU_OBJ_CPU) $(LIB) -o $@ $(NVCC_FLAGS_CPU)

%_cpu.o:%.cu
	$(NVCC) $< -c -o $@ $(NVCC_FLAGS_CPU) $(INCLUDE)

$(EXE)_gpu.x:$(OBJ) $(CU_OBJ_GPU) 
	$(NVCC) $(OBJ) $(CU_OBJ_GPU) $(LIB) -o $@ $(NVCC_FLAGS_GPU) 

%_gpu.o:%.cu
	$(NVCC) $< -c -o $@ $(NVCC_FLAGS_GPU) $(INCLUDE)

%.o:%.cpp
	$(CC) $< -c -o $@ $(CFLAGS) $(INCLUDE)

clean:
	rm -rf $(EXE)_cpu.x $(EXE)_gpu.x $(OBJ) $(CU_OBJ_CPU) $(CU_OBJ_GPU)
