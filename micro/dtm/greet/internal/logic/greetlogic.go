package logic

import (
	"context"
	"database/sql"
	"fmt"

	"study/micro/dtm/greet/internal/svc"
	"study/micro/dtm/greet/internal/types"
	"study/micro/dtm/rpc/bus"

	_ "github.com/dtm-labs/driver-gozero"
	"github.com/dtm-labs/dtmcli"
	"github.com/dtm-labs/dtmcli/dtmimp"
	"github.com/dtm-labs/dtmgrpc"
	"github.com/tal-tech/go-zero/core/logx"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/internal/status"
)

type GreetLogic struct {
	logx.Logger
	ctx    context.Context
	svcCtx *svc.ServiceContext
}

func NewGreetLogic(ctx context.Context, svcCtx *svc.ServiceContext) GreetLogic {
	return GreetLogic{
		Logger: logx.WithContext(ctx),
		ctx:    ctx,
		svcCtx: svcCtx,
	}
}

func (l *GreetLogic) Greet(req types.Request) (*types.Response, error) {
	busServer, err := l.svcCtx.Config.BusRpc.BuildTarget()
	if err != nil {
		return nil, err
	}

	l.msg(busServer)
	l.tcc(busServer)
	l.saga(busServer)

	return &types.Response{}, nil
}

func (l *GreetLogic) msg(server string) string {
	gid := dtmgrpc.MustGenGid(l.svcCtx.Config.DtmServer)
	msg := dtmgrpc.NewMsgGrpc(l.svcCtx.Config.DtmServer, gid).
		Add(server+"/bus.Bus/Ping", &bus.Request{Ping: "hi dtm msg 1!"}). // 方法1
		Add(server+"/bus.Bus/Ping", &bus.Request{Ping: "hi dtm msg 2!"})  // 方法2

	err := msg.Submit()
	dtmimp.FatalIfError(err)
	return gid
}

func (l *GreetLogic) tcc(server string) string {
	gid := dtmgrpc.MustGenGid(l.svcCtx.Config.DtmServer)
	err := dtmgrpc.TccGlobalTransaction(l.svcCtx.Config.DtmServer, gid, func(tcc *dtmgrpc.TccGrpc) error {
		rep := bus.Response{}
		err := tcc.CallBranch(&bus.Request{Ping: "hi dtm tcc!"},
			server+"/bus.Bus/Ping", // try
			server+"/bus.Bus/Ping", // confirm
			server+"/bus.Bus/Ping", // cancel
			&rep)

		if err != nil {
			return err
		}

		err = tcc.CallBranch(&bus.Request{Ping: "hi dtm tcc!"},
			server+"/bus.Bus/Ping",
			server+"/bus.Bus/Ping",
			server+"/bus.Bus/Ping",
			&rep)

		return err
	})

	dtmimp.FatalIfError(err)

	return gid
}

func (l *GreetLogic) saga(server string) string {
	gid := dtmgrpc.MustGenGid(l.svcCtx.Config.DtmServer)
	saga := dtmgrpc.NewSagaGrpc(l.svcCtx.Config.DtmServer, gid).
		Add(server+"/bus.Bus/Ping", server+"/bus.Bus/Ping", &bus.Request{Ping: "hi dtm saga 1"}).
		Add(server+"/bus.Bus/Ping", server+"/bus.Bus/Ping", &bus.Request{Ping: "hi dtm saga 2"})
	err := saga.Submit()

	fmt.Printf("saga submit err:%v", err)

	return saga.Gid
}

func (l *GreetLogic) xa(server string) string {
	gid := dtmgrpc.MustGenGid(l.svcCtx.Config.DtmServer)
	dbConf := dtmimp.DBConf{
		Driver:   "mysql",
		Host:     "127.0.0.1",
		Port:     3306,
		User:     "root",
		Password: "root",
	}
	xaGrpcClient := dtmgrpc.NewXaGrpcClient(l.svcCtx.Config.DtmServer, dbConf, server+"/bus.Bus/XaNotify")
	req := &bus.Request{Ping: "hi dtm saga 1"}
	xaGrpcClient.XaGlobalTransaction(gid, func(xa *dtmgrpc.XaGrpc) error {
		res := &bus.Response{}
		err := xa.CallBranch(req, server+"/bus.Bus/Ping", res)
		if err != nil {
			return err
		}

		err = xa.CallBranch(req, server+"/bus.Bus/Ping", res)
		return err
	})

	return gid
}

func xaLoalTransaction(ctx context.Context, xaGrpcClient *dtmgrpc.XaGrpcClient, req *bus.Request) {
	xaGrpcClient.XaLocalTransaction(ctx, req, func(db *sql.DB, xa *dtmgrpc.XaGrpc) error {
		if req.Ping == "failure" {
			return status.New(codes.Aborted, dtmcli.ResultFailure).Err()
		}

		_, err := dtmimp.DBExec(db, "update abc set aa=1")
		return err
	})
}
