// Function to find maximum product subset
function findMaxProduct(arr) {
    let n = arr.length;

    // If only one element
    if (n === 1) return arr[0];

    let ans = 1;
    let mod = 1e9 + 7;

    let zeroCount = 0, negCount = 0;
    let maxNeg = Number.MIN_SAFE_INTEGER, idxMaxNeg = -1;

    // Count zeros and negatives
    for (let i = 0; i < n; i++) {
        if (arr[i] === 0) {
            zeroCount++;
        } else if (arr[i] < 0) {
            negCount++;
            if (idxMaxNeg === -1 || arr[i] > maxNeg) {
                maxNeg = arr[i];
                idxMaxNeg = i;
            }
        }
    }

    // Edge cases
    if (zeroCount === n) return 0;
    if (negCount === 1 && zeroCount === n - 1) return 0;

    // Calculate product
    for (let i = 0; i < n; i++) {
        if (arr[i] === 0) continue;

        // Skip one negative if count is odd
        if (negCount % 2 === 1 && i === idxMaxNeg) continue;

        ans = ((ans * arr[i]) % mod + mod) % mod;
    }

    return ans;
}

// Driver Code
let arr = [ -1, -1, -2, 4, 3 ];
console.log(findMaxProduct(arr));
