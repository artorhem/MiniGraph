#ifndef MINIGRAPH_DATA_MNGR_H
#define MINIGRAPH_DATA_MNGR_H

#include "utility/io/csr_io_adapter.h"
#include <folly/AtomicHashMap.h>
#include <memory>

namespace minigraph {
namespace utility {
namespace io {

template <typename GID_T, typename VID_T, typename VDATA_T, typename EDATA_T>
class DataMgnr {
  using GRAPH_BASE_T = graphs::Graph<GID_T, VID_T, VDATA_T, EDATA_T>;
  using CSR_T = graphs::ImmutableCSR<GID_T, VID_T, VDATA_T, EDATA_T>;
  using MSG_T = graphs::Message<VID_T, VDATA_T, EDATA_T>;
  using VertexInfo = graphs::VertexInfo<VID_T, VDATA_T, EDATA_T>;

 public:
  DataMgnr() {
    pgraph_by_gid_ =
        std::make_unique<folly::AtomicHashMap<GID_T, GRAPH_BASE_T*>>(1024);

    csr_io_adapter_ = std::make_unique<
        utility::io::CSRIOAdapter<GID_T, VID_T, VDATA_T, EDATA_T>>();
  };

  bool LoadGraph(GID_T gid, CSRPt csr_pt) {
    auto immutable_csr =
        new graphs::ImmutableCSR<GID_T, VID_T, VDATA_T, EDATA_T>;
    LOG_INFO("LOADGRAPH");
    immutable_csr->Deserialized();
    immutable_csr->ShowGraph();
    if (csr_io_adapter_->Read((GRAPH_BASE_T*)immutable_csr, csr_bin, gid,
                              csr_pt.vertex_pt, csr_pt.meta_in_pt,
                              csr_pt.meta_out_pt, csr_pt.vdata_pt,
                              csr_pt.localid2globalid_pt)) {
      pgraph_by_gid_->insert(gid, (GRAPH_BASE_T*)immutable_csr);
      return true;
    } else {
      return false;
    }
  }

  bool WriteGraph(GID_T gid, CSRPt csr_pt) {
    auto graph = this->GetGraph(gid);
    if (csr_io_adapter_->Write(*((GRAPH_BASE_T*)graph), csr_bin,
                               csr_pt.vertex_pt, csr_pt.meta_in_pt,
                               csr_pt.meta_out_pt, csr_pt.vdata_pt,
                               csr_pt.localid2globalid_pt)) {
      return true;
    } else {
      return false;
    }
  }

  GRAPH_BASE_T* GetGraph(GID_T gid) {
    if (pgraph_by_gid_->count(gid)) {
      return pgraph_by_gid_->find(gid)->second;
    } else {
      return nullptr;
    }
  }

  // folly::AtomicHashMap<VID_T, VertexInfo*>* LoadBorderVertexes(
  //     const std::string& border_vertexes_pt) {
  //   folly::AtomicHashMap<VID_T, VertexInfo*>* global_border_vertexes =
  //   // csr_io_adapter_->ReadGlobalBorderVertexes(global_border_vertexes,
  //   //                                           border_vertexes_pt);
  //   return global_border_vertexes;
  // }

  void EraseGraph(GID_T gid) { pgraph_by_gid_->erase(gid); };

  void EraseMsg(){};
  void InsertMsg(){};

 private:
  std::unique_ptr<MSG_T> global_msg_ = nullptr;
  std::unique_ptr<folly::AtomicHashMap<GID_T, GRAPH_BASE_T*>> pgraph_by_gid_ =
      nullptr;
  std::unique_ptr<utility::io::CSRIOAdapter<GID_T, VID_T, VDATA_T, EDATA_T>>
      csr_io_adapter_;
};

}  // namespace io
}  // namespace utility
}  // namespace minigraph
#endif  // MINIGRAPH_DATA_MNGR_H
