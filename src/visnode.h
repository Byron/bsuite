#ifndef PTEX_VISUALIZATION_NODE
#define PTEX_VISUALIZATION_NODE

#include <Ptexture.h>
#include <maya/MPxLocatorNode.h>

typedef PtexPtr<PtexFilter> PtexFilterPtr;
typedef PtexPtr<PtexTexture> PtexTexturePtr;

class PtexVisNode : public MPxLocatorNode
{
	public:
		PtexVisNode();
		virtual ~PtexVisNode();

		virtual MStatus compute(const MPlug&, MDataBlock&);
		virtual void    postConstructor();
		virtual void	draw(M3dView &view, const MDagPath &path, M3dView::DisplayStyle style, M3dView::DisplayStatus);

		static  void*   creator();
		static  MStatus initialize();

		static const MTypeId typeId;				//!< binary file type id
		static const MString typeName;				//!< node type name

	protected:
		//! \return filter type enumeration based on an int (in a safe manner as we do not cast)
		static PtexFilter::FilterType to_filter_type(int type);
		
		//! Assure a filter is present.
		//! \return false if this is not the case, usually because the filename is invalid
		bool assure_filter(MDataBlock& data);
		
	protected:
		// Input attributes
		static MObject aPtexFileName;			//!< path to ptex file to use
		static MObject aPtexFilterType;			//!< type of filter for evaluation
		static MObject aPtexFilterSize;			//!< desired uv filter size
		static MObject aInMesh;					//!< mesh to display the ptex information for
		
		// output attributes
		static MObject aMetaDataKeys;			//!< string array of meta data keys
		static MObject aNeedsCompute;			//!< dummy output (for now) to check if we need to compute
		

	protected:
		uint32_t		m_ptex_num_channels;	//!< Amount of channels currently stored in the file
		PtexFilterPtr	m_ptex_filter;			//!< filter ptr to be used for ptex evaluation
		PtexTexturePtr	m_ptex_texture;			//!< texture pointer
		MString			m_error;				//!< error string
};

#endif
