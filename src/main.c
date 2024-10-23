#include <openjpeg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Callback functions for OpenJPEG */
void error_callback(const char *msg, void *client_data)
{
    fprintf(stderr, "ERROR: %s\n", msg);
}

void warning_callback(const char *msg, void *client_data)
{
    fprintf(stderr, "WARNING: %s\n", msg);
}

void info_callback(const char *msg, void *client_data)
{
    fprintf(stdout, "INFO: %s\n", msg);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <input.jp2>\n", argv[0]);
        return 1;
    }

    const char *input_filename = argv[1];

    /* Set up the decoder parameters */
    opj_dparameters_t parameters;
    opj_set_default_decoder_parameters(&parameters);

    /* Create a stream from the file */
    opj_stream_t *stream = opj_stream_create_default_file_stream(input_filename, 1);
    if (!stream)
    {
        fprintf(stderr, "Failed to create input stream\n");
        return 1;
    }

    /* Create a codec based on the file type (JP2 in this case) */
    opj_codec_t *codec = opj_create_decompress(OPJ_CODEC_JP2);
    if (!codec)
    {
        fprintf(stderr, "Failed to create codec\n");
        opj_stream_destroy(stream);
        return 1;
    }

    /* Set up the event manager */
    opj_set_info_handler(codec, info_callback, NULL);
    opj_set_warning_handler(codec, warning_callback, NULL);
    opj_set_error_handler(codec, error_callback, NULL);

    /* Setup the decoder with the parameters */
    if (!opj_setup_decoder(codec, &parameters))
    {
        fprintf(stderr, "Failed to set up decoder\n");
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        return 1;
    }

    /* Read the header to get image information */
    opj_image_t *image = NULL;
    if (!opj_read_header(stream, codec, &image))
    {
        fprintf(stderr, "Failed to read the image header\n");
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        return 1;
    }

    opj_codestream_info_v2_t *cstr_info = opj_get_cstr_info(codec);

    OPJ_UINT32 image_width = image->comps[0].w;
    OPJ_UINT32 image_height = image->comps[0].h;

    OPJ_UINT32 total_tiles = cstr_info->tw * cstr_info->th;

    /* Print image and tile information */
    printf("Image info:\n");
    printf("Width: %d, Height: %d\n", image_width, image_height);
    printf("Number of tiles (X, Y): (%d, %d)\n", cstr_info->tw, cstr_info->th);
    printf("Total number of tiles: %d\n", total_tiles);

    /* Decode each tile */
    for (OPJ_UINT32 tile_index = 0; tile_index < total_tiles; tile_index++)
    {
        printf("Decoding tile %d...\n", tile_index);
        if (!opj_get_decoded_tile(codec, stream, image, tile_index))
        {
            fprintf(stderr, "Failed to decode tile %d. Quitting.\n", tile_index);
            break;
        }
        else
        {
            printf("Tile %d decoded successfully.\n", tile_index);
        }
    }

    /* Clean up */
    opj_image_destroy(image);
    opj_stream_destroy(stream);
    opj_destroy_cstr_info(&cstr_info);
    opj_destroy_codec(codec);

    return 0;
}
