// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

//   Configuration for multiple platforms/environments
//
// Date: 8/3/97
// Author: Matt Wilhelm

// want to see debugging crap?
#define i3d_DEBUG

// Select a platform
#define i3d_UNIX
#undef i3d_DOS
#undef i3d_WINDOWS

// (optional) Select a flavor
#define i3d_LINUX
#undef i3d_IRIX    // examples only. Not implemented
#undef i3d_SOLARIS

#ifdef i3d_UNIX
// use X shared memory extension? It is faster but may not be available
#define SHM
#endif
