#include "KL3DSliceDragger.h"

BEGIN_KLDISPLAY3D_NAMESPACE

KL3DSliceNode * KL3DSliceDragger::_activeSlice = NULL;

KL3DSliceDragger::KL3DSliceDragger(void)
{
	_activeSlice = NULL;
}


KL3DSliceDragger::~KL3DSliceDragger(void)
{
}


void KL3DSliceDragger::setDirection(float direct[3])
{
	osg::Vec3d startL = osg::Vec3d(0,0,0);
	osg::Vec3d endL = osg::Vec3d(direct[0], direct[1],direct[2]);
	_projector->setLine(startL,endL);
}

bool KL3DSliceDragger::handle(const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	// Check if the dragger node is in the nodepath.
	if (_checkForNodeInNodePath)
	{
		if (!pointer.contains(this)) return false;
	}		
	switch (ea.getEventType())
	{
		// Pick start.
	case (osgGA::GUIEventAdapter::PUSH):
		{
			_orig.set(_slice->getOrigin()[0],_slice->getOrigin()[1],_slice->getOrigin()[2]);
			setDirection(_slice->getNormal());
			// Get the LocalToWorld matrix for this node and set it for the projector.
			osg::NodePath nodePathToRoot;
			computeNodePathToRoot(*this,nodePathToRoot);
			osg::Matrix localToWorld = osg::computeLocalToWorld(nodePathToRoot);
			_projector->setLocalToWorld(localToWorld);

			if (_projector->project(pointer, _startProjectedPoint))
			{
				// Generate the motion command.
				osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(_projector->getLineStart(),
					_projector->getLineEnd());
				cmd->setStage(MotionCommand::START);
				cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());

				// Dispatch command.
				dispatch(*cmd);
				if (_activeSlice != NULL)
					_activeSlice->setActiveOff();
				_activeSlice = _slice;
				_activeSlice->setActiveOn();
				aa.requestRedraw();
			}
			return true;
		}

		// Pick move.
	case (osgGA::GUIEventAdapter::DRAG):
		{
			osg::Vec3d projectedPoint;
			const float *bounds = _slice->getBounds();
			if (_projector->project(pointer, projectedPoint))
			{
				osg::Vec3d v = osg::Vec3(projectedPoint - _startProjectedPoint);
				// Generate the motion command.					
				osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(_projector->getLineStart(),_projector->getLineEnd());
				cmd->setStage(MotionCommand::MOVE);
				cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());
				cmd->setTranslation(projectedPoint - _startProjectedPoint);
				dispatch(*cmd);
				//```````````````set the origin

				DraggerCallback *dc = _selfUpdater;
				DraggerTransformCallback *dtc= dynamic_cast<DraggerTransformCallback*>(dc);
				osg::MatrixTransform *trans = dtc->getTransform();
				osg::Matrix tranMatrix = trans->getMatrix();
				_origNow = _orig * tranMatrix;//点下鼠标左键时的原点乘以变换矩阵

				for(int d = 0;d < 3;d ++)
				{
					if(_origNow[d] <= bounds[d])
					{
						_origNow[d] = bounds[d] + 1;
						break;
					}
					else if (_origNow[d] >= bounds[d+3])
					{
						_origNow[d] = bounds[d+3] - 1;
						break;
					}
				}
				_slice->setOrigin(_origNow[0],_origNow[1],_origNow[2]);
				_slice->updateDrag();
				//_slice->updatePosition();
				trans->setMatrix(osg::Matrix::translate(0,0,0));//!!!!!!!!!阻止切片随着dragger移动
				aa.requestRedraw();
			}
			return true;
		}

		// Pick finish.
	case (osgGA::GUIEventAdapter::RELEASE):
		{
			osg::Vec3d projectedPoint;
			if (_projector->project(pointer, projectedPoint))
			{
				osg::ref_ptr<TranslateInLineCommand> cmd = new TranslateInLineCommand(_projector->getLineStart(),
					_projector->getLineEnd());

				cmd->setStage(MotionCommand::FINISH);
				cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());

				_slice->updateRelease();
				// Dispatch command.
				dispatch(*cmd);
				aa.requestRedraw();
			}
			return true;
		}
	default:
		return false;
	}
}

void KL3DSliceDragger::setupDefaultGeometry()
{
	addChild(_slice);
}

void KL3DSliceDragger::setSlice(KL3DSliceNode * slice)
{
	_slice = slice;
	setDirection(_slice->getNormal());
	setupDefaultGeometry();
}

END_KLDISPLAY3D_NAMESPACE