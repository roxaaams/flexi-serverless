// Function to integrate
function myFunction(x) {
    return Math.pow(x, 4) * Math.exp(-x);
}

exports.handler = async (event) => {
    // Retrieve the payload from the event object
    const payload = event;

    try {
        // Process the payload or perform any desired operations
        console.log('Received payload:', payload);

        const str1 = payload.str1;
        const str2 = payload.str2;

        const len1 = str1.length;
        const len2 = str2.length;

        const dp = new Array(len1 + 1).fill().map(() => new Array(len2 + 1).fill(0));

        for (let i = 0; i <= len1; ++i) {
            dp[i][0] = i;
        }
        for (let j = 0; j <= len2; ++j) {
            dp[0][j] = j;
        }

        for (let i = 1; i <= len1; ++i) {
            for (let j = 1; j <= len2; ++j) {
                const cost = str1[i - 1] === str2[j - 1] ? 0 : 1;
                dp[i][j] = Math.min(dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost);
            }
        }


        // Return a response
        const response = {
            statusCode: 200,
            body: JSON.stringify({ distance: dp[len1][len2] }),
        };
        return response;
    } catch (error) {
        // Handle any errors that occur during processing
        console.error('Error processing payload:', error);

        // Return an error response
        const response = {
            statusCode: 500,
            body: JSON.stringify({ message: error.message || 'An error occurred during function execution' }),
        };
        return response;
    }

};
