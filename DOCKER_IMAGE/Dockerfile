# Stage 1: Build the C application
# We use the 'latest' tag to get the most recent version of the gcc image.
# Using a multi-stage build helps in keeping the final image small.
FROM gcc:latest AS builder

# Set the working directory inside the container
WORKDIR /app

# Copy your specific source and header files into the container's /app directory
COPY VirtualFileSystem.h VFS_Functions.c VirtualFileSystem.c ./

# Compile your C code using the specific command you provided.
# The output executable will be named 'vfs'.
RUN gcc VFS_Functions.c VirtualFileSystem.c -o vfs

# Stage 2: Create the final, smaller image
# We use debian:bookworm-slim, which has a newer glibc version
# that is compatible with the one used by gcc:latest.
FROM debian:bookworm-slim

# Set the working directory in the final image
WORKDIR /app

# Copy only the compiled executable 'vfs' from the 'builder' stage
COPY --from=builder /app/vfs .

# Set the command to run when the container starts.
# This will execute your virtual file system program.
CMD ["./vfs"]
