import numpy as np


# This function performs least-squares polynomial regression
# of degree model_degree on the given x and y point arrays, then
# evaluates the polynomial at predict_x to return a prediction.
# See https://mathworld.wolfram.com/LeastSquaresFittingPolynomial.html
# for more information (step 16 contains the relevant formula).
def poly_predict(x: np.ndarray, y: np.ndarray, model_degree: int, predict_x: float):
    # Note the transpose operation: the x ** n values are the columns of the matrix
    # in step 13.
    x_mat = np.transpose(np.array([x ** n for n in range(model_degree + 1)]))

    x_mat_t = np.transpose(x_mat)
    coeffs = np.matmul(np.matmul(
        np.linalg.inv(np.matmul(x_mat_t, x_mat)),
        x_mat_t),
        y)

    predict_pt_powers = np.array([predict_x ** n for n in range(model_degree + 1)])
    return np.dot(predict_pt_powers, coeffs).item()


if __name__ == '__main__':
    test_x = np.array([0.0, 2.0, 2.5, 6.0])
    test_y = np.array([3.4, 7.5, 9.2, 28.9])

    print(f'Quadratic prediction at 5.0: {poly_predict(test_x, test_y, 2, 5.0)}')
