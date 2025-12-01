# syntax=docker/dockerfile:1.7-labs
FROM archlinux:base-20251019.0.436919 as builder

# Parallel build jobs, builder login name, and CPM cache location.
ARG J=1
ARG BUILDER=builder
ARG CPM_SOURCE_CACHE=/home/${BUILDER}/.cache

# Install system packages required for building.
RUN --mount=type=cache,target=/var/cache/pacman/pkg pacman -Sy --noconfirm && \
	pacman -S --noconfirm sudo gcc clang libedit llvm-libs compiler-rt libisl \
		libmpc pkgconf make cmake python snappy libuv rhash cppdap git gflags \
		jemalloc jsoncpp ninja

# Ensure Perl core binaries are available (pod2man).
ENV PATH=/usr/bin/core_perl:$PATH

# Add a builder user with passwordless sudo.
RUN useradd -m "${BUILDER}" && \
	echo "${BUILDER} ALL=(ALL) NOPASSWD: ALL" > "/etc/sudoers.d/${BUILDER}"

# Switch to the builder user.
USER ${BUILDER}

# Set the working directory for the project sources.
WORKDIR /home/${BUILDER}/src

# Copy the project source tree.
COPY . .

# Build project sources with caching support.
RUN --mount=type=cache,target=${CPM_SOURCE_CACHE},uid=1000,gid=1000,mode=0755 \
	make PACENV_DIR= CPM_SOURCE_CACHE=${CPM_SOURCE_CACHE} J=${J} compile

# Final runtime image based on Arch Linux.
FROM archlinux:base-20251019.0.436919

# Install only runtime dependencies needed to run the atlas binary.
RUN pacman -Sy --noconfirm && pacman -S --noconfirm jemalloc snappy

# Copy the compiled atlas binary from the builder stage.
COPY --from=builder /home/builder/src/build/server /usr/local/bin/

# Set atlas as the default entrypoint for the container.
ENTRYPOINT ["server"]

# vim: set ts=4 sw=4 noexpandtab:

