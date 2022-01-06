package logic

import (
	"context"

	"study/micro/dtm/rpc/bus"
	"study/micro/dtm/rpc/internal/svc"

	"github.com/tal-tech/go-zero/core/logx"
)

type XaNotifyLogic struct {
	ctx    context.Context
	svcCtx *svc.ServiceContext
	logx.Logger
}

func NewXaNotifyLogic(ctx context.Context, svcCtx *svc.ServiceContext) *XaNotifyLogic {
	return &XaNotifyLogic{
		ctx:    ctx,
		svcCtx: svcCtx,
		Logger: logx.WithContext(ctx),
	}
}

func (l *XaNotifyLogic) XaNotify(in *bus.XaRequest) (*bus.XaResponse, error) {
	// todo: add your logic here and delete this line

	return &bus.XaResponse{}, nil
}
