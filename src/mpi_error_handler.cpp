#include <gdsb/mpi_error_handler.h>

#include <stdexcept>
#include <string>

#include <mpi.h>

//! Error messages are copied from MPICH documentation:
//! https://www.mpich.org/static/docs/latest/www3/MPI_Type_create_struct.html
void gdsb::mpi::handle_type_create_struct_error(int const ec)
{
    std::string error_message;
    switch (ec)
    {
    case MPI_SUCCESS:
        // No error; MPI routine completed successfully.
        return;
    case MPI_ERR_ARG:
        error_message = "Invalid argument. Some argument is invalid and is not identified by a specific error class "
                        "(e.g., MPI_ERR_RANK).";
        break;
    case MPI_ERR_COUNT:
        error_message =
            "Invalid count argument. Count arguments must be non - negative; a count of zero is often valid";
        break;
    case MPI_ERR_TYPE:
        error_message = "Invalid datatype argument.";
        break;
    case MPI_ERR_OTHER:
        error_message = "Other error.";
        break;
    }

    throw std::runtime_error(error_message.c_str());
}
