from typing import Optional

import numpy as np


# This function performs least-squares polynomial regression
# of degree model_degree on the given x and y point arrays, then
# evaluates the polynomial at predict_x to return a prediction.
# See https://mathworld.wolfram.com/LeastSquaresFittingPolynomial.html
# for more information (step 16 contains the relevant formula).
def poly_predict(x: np.ndarray, y: np.ndarray, model_degree: int, predict_x: float,
                 weights: Optional[np.ndarray] = None): # model_degree defines degree of polynomial regression.
    assert x.shape == y.shape
    assert x.shape[0] >= model_degree + 1  # Too few points results in an ambiguous result/non-invertible matrix

    # Note the transpose operation: the x ** n values are the columns of the matrix
    # in step 13.
    x_mat = np.transpose(np.array([x ** n for n in range(model_degree + 1)])) # creates a matrix where each column corresponds to x^0 to x^model_degree

    x_mat_t = np.transpose(x_mat)

    if weights is None:
        coeffs = np.matmul(np.matmul(
            np.linalg.inv(np.matmul(x_mat_t, x_mat)),
            x_mat_t),
            y)
    else:
        # Weighted least squares; the calculation is similar, but uses a diagonal
        # weight matrix. See https://online.stat.psu.edu/stat501/lesson/13/13.1 for more information.
        weight_mat = np.diag(weights)
        coeffs = np.matmul(np.matmul(np.matmul(
            np.linalg.inv(np.matmul(np.matmul(x_mat_t, weight_mat), x_mat)),
            x_mat_t),
            weight_mat),
            y)

    predict_pt_powers = np.array([predict_x ** n for n in range(model_degree + 1)]) # creates the x^0 + x^1 + x^2 + ... + x^model_degree array
    return np.dot(predict_pt_powers, coeffs).item() # dot product of x with coefficients gives the y values


if __name__ == '__main__':
    test_x = np.array([0.0, 2.0])
    test_y = np.array([0.0, 7.5])

    print(f'Quadratic prediction at 5.0: {poly_predict(test_x, test_y, 1, 4.0)}')

# poly_predict is used in ArmorDetect_D435i.py where the input is hisotrical time data points (time_hist_array) , 
# x_hist_array corresponds to values observed at each time point in time_hist_array 
# and gives a single predicted value that forecasts future position state

