const Jimp = require("jimp");

exports.handler = async (event) => {
    const imagePairs = event.imagePairs; // Assuming an array of objects with { id, imageUrl }

    try {
        // Create an array of promises, each promise processes an image
        const imageProcessingPromises = imagePairs.map(async (pair) => {
            const { id, imageUrl } = pair;

            try {
                const image = await Jimp.read({
                    url: imageUrl, // Required!
                    headers: {},
                });

                let blackPixels = 0;
                let whitePixels = 0;

                // Scan through the image
                image.scan(0, 0, image.bitmap.width, image.bitmap.height, function (x, y, idx) {
                    var red = this.bitmap.data[idx + 0];
                    var green = this.bitmap.data[idx + 1];
                    var blue = this.bitmap.data[idx + 2];

                    // Check if the pixel is a shade of gray (red == green == blue)
                    if (red === green && green === blue) {
                        if (red === 0) {
                            blackPixels++;
                        } else if (red === 255) {
                            whitePixels++;
                        }
                    }
                });

                // Return the result for the current image pair
                return {
                    id,
                    blackPixels,
                    whitePixels,
                };

            } catch (error) {
                console.error(`Error processing image with id ${id}:`, error);
                return {
                    id,
                    error: 'Error processing image'
                };
            }
        });

        // Wait for all the image processing promises to resolve
        const results = await Promise.all(imageProcessingPromises);

        // Return the array of results
        return {
            statusCode: 200,
            body: JSON.stringify({ results }),
        };

    } catch (error) {
        console.error('Error processing images:', error);
        return {
            statusCode: 500,
            body: 'Error processing images',
        };
    }
};
