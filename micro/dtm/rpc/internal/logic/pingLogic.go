package logic

import (
	"context"

	"study/micro/dtm/rpc/bus"
	"study/micro/dtm/rpc/internal/svc"

	"github.com/tal-tech/go-zero/core/logx"
)

type PingLogic struct {
	ctx    context.Context
	svcCtx *svc.ServiceContext
	logx.Logger
}

func NewPingLogic(ctx context.Context, svcCtx *svc.ServiceContext) *PingLogic {
	return &PingLogic{
		ctx:    ctx,
		svcCtx: svcCtx,
		Logger: logx.WithContext(ctx),
	}
}

func (l *PingLogic) Ping(in *bus.Request) (*bus.Response, error) {
	// todo: add your logic here and delete this line

	return &bus.Response{}, nil
}
