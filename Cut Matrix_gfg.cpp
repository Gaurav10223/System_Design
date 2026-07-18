let mod = 1000000007;
// potd gfg
function solve(r, c, k, n, m, suf) {

    // No 1s in remaining submatrix
    if (suf[r][c] === 0)
        return 0;

    // Last piece must have at least one 1
    if (k === 1)
        return 1;

    let res = 0;

    // Try all horizontal cuts
    for (let i = r; i < n - 1; i++)
        if (suf[r][c] - suf[i + 1][c] > 0)
            res = (res + solve(i + 1, c, k - 1, n, m, suf)) % mod;

    // Try all vertical cuts
    for (let j = c; j < m - 1; j++)
        if (suf[r][c] - suf[r][j + 1] > 0)
            res = (res + solve(r, j + 1, k - 1, n, m, suf)) % mod;

    return res;
}

function findWays(matrix, k) {
    let n = matrix.length;
    let m = matrix[0].length;

    // Build suffix sum
    let suf = Array.from({length: n + 1}, () => new Array(m + 1).fill(0));
    for (let i = n - 1; i >= 0; i--)
        for (let j = m - 1; j >= 0; j--)
            suf[i][j] = suf[i + 1][j] + suf[i][j + 1] - suf[i + 1][j + 1] + matrix[i][j];

    return solve(0, 0, k, n, m, suf);
}

// Driver code
console.log(findWays([[1, 0, 0], [1, 1, 1], [0, 0, 0]], 3));
