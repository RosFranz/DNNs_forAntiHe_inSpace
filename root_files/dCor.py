import pandas as pd
import dcor

df = pd.read_csv("xy.csv", header=None)
x = df[0].values
y = df[1].values
CL = df[2].values
AE = df[3].values

dc1 = dcor.distance_correlation(x, y)
dc2 = dcor.distance_correlation(CL, AE)
print("Distance Correlation Mass-Beta: ", dc1, ", CL-AE: ",dc2 )
