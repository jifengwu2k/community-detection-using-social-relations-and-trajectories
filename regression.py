from scipy.optimize import curve_fit


def regression(f, x, y):
    # calculate parameter_values
    parameter_values, *_ = curve_fit(f, x, y)
    
    # calculate r_squared
    residual_square_sum = sum((y - f(x, *parameter_values)) ** 2)
    total_square_sum = sum((y - y.mean()) ** 2)
    r_squared = 1 - residual_square_sum / total_square_sum
    
    return parameter_values, r_squared
