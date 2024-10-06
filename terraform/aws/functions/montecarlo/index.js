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

        const results = await Promise.all(payload.map(async (item) => {
            const { id, iterations, lowBound, upBound } = item;

            if (iterations === undefined || iterations === null || iterations === ''
                || lowBound === undefined || lowBound === null || lowBound === ''
                || upBound === undefined || upBound === null || upBound === '') {
                throw new Error('Iterations, lowBound, and upBound must be defined');
            }

            let totalSum = 0;
            let totalSumSquared = 0;

            let iter = 0;

            while (iter < iterations - 1) {
                const randNum = lowBound + Math.random() * (upBound - lowBound);

                const functionVal = myFunction(randNum);

                totalSum += functionVal;
                totalSumSquared += Math.pow(functionVal, 2);

                iter++;
            }

            const estimate = (upBound - lowBound) * totalSum / iterations;
            const expected = totalSum / iterations;

            const expectedSquare = totalSumSquared / iterations;

            const std = (upBound - lowBound) * Math.sqrt((expectedSquare - Math.pow(expected, 2)) / (iterations - 1));

            return { id, estimate, std };
        }));

        // Return a response
        return {
            statusCode: 200,
            body: JSON.stringify({ results }),
        };
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
