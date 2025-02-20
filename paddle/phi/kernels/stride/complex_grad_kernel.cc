// Copyright (c) 2023 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "paddle/phi/kernels/complex_grad_kernel.h"
#include "paddle/phi/backends/all_context.h"
#include "paddle/phi/common/type_traits.h"
#include "paddle/phi/core/kernel_registry.h"
#include "paddle/phi/kernels/complex_kernel.h"
#include "paddle/phi/kernels/fill_kernel.h"
#include "paddle/phi/kernels/strided_copy_kernel.h"

namespace phi {

template <typename T, typename Context>
void RealGradStridedKernel(const Context& dev_ctx,
                           const DenseTensor& dout,
                           DenseTensor* dx) {
  dev_ctx.Alloc(dx, dx->dtype());
  dx->set_strides(DenseTensorMeta::calc_strides(dx->dims()));
  PD_VISIT_ALL_TYPES(dx->dtype(), "RealGradStridedKernel", ([&] {
                       phi::FillKernel<data_t, Context>(dev_ctx, *dx, 0, dx);
                     }));
  DenseTensor tmp;
  tmp.set_meta(dout.meta());
  RealStridedKernel<T, Context>(dev_ctx, *dx, &tmp);
  PD_VISIT_ALL_TYPES(dout.dtype(), "RealGradStridedKernel", ([&] {
                       phi::StridedCopyKernel<data_t, Context>(
                           dev_ctx,
                           dout,
                           phi::vectorize<int64_t>(tmp.dims()),
                           phi::vectorize<int64_t>(tmp.strides()),
                           tmp.offset(),
                           &tmp);
                     }));
}

template <typename T, typename Context>
void ImagGradStridedKernel(const Context& dev_ctx,
                           const DenseTensor& dout,
                           DenseTensor* dx) {
  dev_ctx.Alloc(dx, dx->dtype());
  dx->set_strides(DenseTensorMeta::calc_strides(dx->dims()));
  PD_VISIT_ALL_TYPES(dx->dtype(), "ImagGradStridedKernel", ([&] {
                       phi::FillKernel<data_t, Context>(dev_ctx, *dx, 0, dx);
                     }));

  DenseTensor tmp;
  tmp.set_meta(dout.meta());
  ImagStridedKernel<T, Context>(dev_ctx, *dx, &tmp);
  PD_VISIT_ALL_TYPES(dout.dtype(), "ImagGradStridedKernel", ([&] {
                       phi::StridedCopyKernel<data_t, Context>(
                           dev_ctx,
                           dout,
                           phi::vectorize<int64_t>(tmp.dims()),
                           phi::vectorize<int64_t>(tmp.strides()),
                           tmp.offset(),
                           &tmp);
                     }));
}

}  // namespace phi

PD_REGISTER_KERNEL(real_grad,
                   CPU,
                   STRIDED,
                   phi::RealGradStridedKernel,
                   phi::dtype::complex<float>,
                   phi::dtype::complex<double>) {
  kernel->InputAt(0).SetDataType(phi::dtype::ToReal(kernel_key.dtype()));
}

PD_REGISTER_KERNEL(imag_grad,
                   CPU,
                   STRIDED,
                   phi::ImagGradStridedKernel,
                   phi::dtype::complex<float>,
                   phi::dtype::complex<double>) {
  kernel->InputAt(0).SetDataType(phi::dtype::ToReal(kernel_key.dtype()));
}

#if defined(PADDLE_WITH_CUDA) || defined(PADDLE_WITH_HIP)
PD_REGISTER_KERNEL(real_grad,
                   GPU,
                   STRIDED,
                   phi::RealGradStridedKernel,
                   phi::dtype::complex<float>,
                   phi::dtype::complex<double>) {
  kernel->InputAt(0).SetDataType(phi::dtype::ToReal(kernel_key.dtype()));
}

PD_REGISTER_KERNEL(imag_grad,
                   GPU,
                   STRIDED,
                   phi::ImagGradStridedKernel,
                   phi::dtype::complex<float>,
                   phi::dtype::complex<double>) {
  kernel->InputAt(0).SetDataType(phi::dtype::ToReal(kernel_key.dtype()));
}
#endif
