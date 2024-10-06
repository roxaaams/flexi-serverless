const Jimp = require("jimp");


exports.handler = async (event) => {
    const imageUrl = event.image; // Assuming the image URL is passed as 'imageUrl' in the event object

    try {

        const image = await Jimp.read({
            url: imageUrl, // Required!
            headers: {},

        })
        let blackPixels = 0;
        let whitePixels = 0;

        image.scan(0, 0, image.bitmap.width, image.bitmap.height, function (x, y, idx) {
            // x, y is the position of this pixel on the image
            // idx is the position start position of this rgba tuple in the bitmap Buffer
            // this is the image

            var red = this.bitmap.data[idx + 0];
            var green = this.bitmap.data[idx + 1];
            var blue = this.bitmap.data[idx + 2];
            var alpha = this.bitmap.data[idx + 3];

            // rgba values run from 0 - 255"
            // e.g. this.bitmap.data[idx] = 0; // removes red from this pixel
            if (red === green && green === blue) {
                if (red === 0) {
                    whitePixels++;
                } else if (red === 255) {
                    blackPixels++;
                }
            }
        });

        return {
            statusCode: 200,
            body: JSON.stringify({ blackPixels, whitePixels }),
        };

    } catch (error) {
        console.error('Error processing image:', error);
        return {
            statusCode: 500,
            body: 'Error processing image',
        };
    }
};
